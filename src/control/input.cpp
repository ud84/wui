//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/input.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

#include <wui/system/clipboard_tools.hpp>

#include <wui/locale/locale.hpp>

#include <wui/common/flag_helpers.hpp>

#include <boost/nowide/convert.hpp>
#include <utf8/utf8.h>

#include <regex>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

#undef min
#undef max

namespace wui
{

static const int32_t INPUT_HORIZONTAL_INDENT = 5;
static const int32_t SCROLL_SIZE = 14;

input::input(std::string_view text__, input_view input_view__, input_content input_content__, int32_t symbols_limit_, std::string_view theme_control_name_, std::shared_ptr<i_theme> theme__)
    : input_view_(input_view__),
    input_content_(input_content__),
    symbols_limit(symbols_limit_),
    text_(text__),
    change_callback(),
    tcn(theme_control_name_),
    theme_(theme__),
    position_(),
    cursor_position(0), select_start_position(0), select_end_position(0),
    parent_(),
    my_control_sid(), my_plain_sid(),
    timer_(std::bind(&input::redraw_cursor, this)),
    menu_(std::make_shared<menu>(menu::tc, theme_)),
    vert_scroll(std::make_shared<scroll>(0, 0, orientation::vertical, std::bind(&input::on_vert_scroll, this, std::placeholders::_1, std::placeholders::_2), scroll::tc, theme__)),
    hor_scroll(std::make_shared<scroll>(0, 0, orientation::horizontal, std::bind(&input::on_hor_scroll, this, std::placeholders::_1, std::placeholders::_2), scroll::tc, theme__)),
    scroll_offset_x(0), scroll_offset_y(0),
    showed_(true), enabled_(true), topmost_(false),
    focused_(false),
    cursor_visible(false),
    selecting(false),
    left_shift(0)
{
    if (input_view_ == input_view::multiline) {
        set_text_multiline(text__);
        reset_multiline_state();
    }
    
    // Initialize auto-scroll timer
    auto_scroll_timer_ = std::make_shared<timer>([this]() { on_auto_scroll(); });
    
    menu_->set_items({
            { 0, menu_item_state::normal, locale(tc, cl_cut).data(), "Ctrl+X", nullptr, {}, [this](int32_t i) { buffer_cut(); } },
            { 1, menu_item_state::normal, locale(tc, cl_copy).data(), "Ctrl+C", nullptr, {}, [this](int32_t i) { buffer_copy(); } },
            { 2, menu_item_state::normal, locale(tc, cl_paste).data(), "Ctrl+V", nullptr, {}, [this](int32_t i) { buffer_paste(); } }
        });
}

input::~input()
{
    stop_auto_scroll();
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }
}

int32_t get_text_width(graphic &gr, std::string text, size_t text_length, const font &font_)
{
    if (text.empty() || text_length == 0) return 0;
    text.resize(text_length);
    auto text_rect = gr.measure_text(text, font_);
    return text_rect.right;
}

// Auxiliary function: comparing cursor positions (row, col)
inline bool cursor_less(size_t row1, size_t col1, size_t row2, size_t col2)
{
    return row1 < row2 || (row1 == row2 && col1 < col2);
}

// Auxiliary function: get the number of characters in a UTF-8 string
static size_t utf8_length(const std::string& s)
{
    return utf8::distance(s.begin(), s.end());
}

// Auxiliary function: get an iterator per character by position (character index)
static std::string::iterator utf8_iter_at(std::string& s, size_t char_pos)
{
    auto it = s.begin();
    utf8::advance(it, char_pos, s.end());
    return it;
}

static std::string::const_iterator utf8_iter_at(const std::string& s, size_t char_pos)
{
    auto it = s.begin();
    utf8::advance(it, char_pos, s.end());
    return it;
}

// Auxiliary functions for working with byte indexes (as in single-line mode)
static size_t get_byte_pos_for_char_pos(const std::string& s, size_t char_pos)
{
    if (s.empty())
    {
        return 0;
    }
    
    auto it = s.begin();
    size_t actual_chars = utf8::distance(s.begin(), s.end());
    if (char_pos >= actual_chars)
    {
        return s.size(); // Returning the end of the string
    }
    
    utf8::advance(it, char_pos, s.end());
    return std::distance(s.begin(), it);
}
static size_t get_char_pos_for_byte_pos(const std::string& s, size_t byte_pos)
{
    if (s.empty()) 
    { 
        return 0;
    }
    
    if (byte_pos >= s.size())
    {
        return utf8::distance(s.begin(), s.end());
    }
    
    return utf8::distance(s.begin(), s.begin() + byte_pos);
}

// Auxiliary function for checking the validity of UTF-8 in multiline
static bool check_count_valid_multiline(const std::string& line, size_t count)
{
    if (count == 0 || line.empty())
    {
        return true;
    }

    if (count > line.size())
    {
        return false;
    }

    auto end_it = utf8::find_invalid(line.begin(), line.begin() + count);
    return end_it == line.begin() + count;
}

void input::draw(graphic &gr, rect )
{
    if (!showed_ || position_.width() == 0 || position_.height() == 0 || position_.width() <= INPUT_HORIZONTAL_INDENT * 2)
    {
        return;
    }

    auto control_pos = position();

    auto font_ = theme_font(tcn, tv_font, theme_);
    if (input_view_ == input_view::password)
    {
#ifdef _WIN32
        font_.name = "Courier New";
#elif __linux__
        font_.name = "monospace";
#endif
    }

    auto border_width = theme_dimension(tcn, tv_border_width, theme_);
    int line_height = font_.size;

    /// Draw the frame
    gr.draw_rect(control_pos,
        !focused_ ? theme_color(tcn, tv_border, theme_) : theme_color(tcn, tv_focused_border, theme_),
        theme_color(tcn, tv_background, theme_),
        border_width,
        theme_dimension(tcn, tv_round, theme_));

    if (input_view_ == input_view::multiline)
    {          
        auto content_width = control_pos.width() - border_width * 2 - SCROLL_SIZE;
        auto content_height = control_pos.height() - border_width * 2 - SCROLL_SIZE;

        auto parent__ = parent_.lock(); if (!parent__) return;
        system_context ctx = parent__->context();
        graphic mem_gr(ctx);
        mem_gr.init({ 0, 0,
            position_.width() - (border_width + INPUT_HORIZONTAL_INDENT) * 2,
            position_.height() - (border_width * 2) },
            theme_color(tcn, tv_background, theme_));
        
        // We start from the scroll position
        int y = - (scroll_offset_y % line_height);
        size_t start_line = scroll_offset_y / line_height;
        int visible_bottom = content_height;
        for (size_t i = start_line; i < lines_.size(); ++i)
        {
            int actual_y = y + static_cast<int>(i - start_line) * line_height;
            if (actual_y >= visible_bottom) break;

            // Highlighting the selection
            bool has_sel = false;
            size_t sel_start = 0, sel_end = 0;
            if (!(select_start_row == select_end_row && select_start_col == select_end_col))
            {
                size_t srow = select_start_row, scol = select_start_col, erow = select_end_row, ecol = select_end_col;
                if (cursor_less(erow, ecol, srow, scol)) { std::swap(srow, erow), std::swap(scol, ecol); }
                if (i > srow && i < erow)
                {
                    has_sel = true; sel_start = 0; sel_end = utf8::distance(lines_[i].begin(), lines_[i].end());
                }
                else if (i == srow && i == erow && scol != ecol)
                {
                    has_sel = true; sel_start = scol; sel_end = ecol;
                    if (sel_start > sel_end) std::swap(sel_start, sel_end);
                }
                else if (i == srow && i < erow)
                {
                    has_sel = true; sel_start = scol; sel_end = utf8::distance(lines_[i].begin(), lines_[i].end());
                }
                else if (i == erow && i > srow)
                {
                    has_sel = true; sel_start = 0; sel_end = ecol;
                }
            }
            if (has_sel && sel_start < sel_end && static_cast<size_t>(sel_end) <= static_cast<size_t>(utf8::distance(lines_[i].begin(), lines_[i].end())))
            {
                size_t start_byte = get_byte_pos_for_char_pos(lines_[i], sel_start);
                size_t end_byte = get_byte_pos_for_char_pos(lines_[i], sel_end);
                int x1 = mem_gr.measure_text(lines_[i].substr(0, start_byte), font_).right - scroll_offset_x;
                int x2 = mem_gr.measure_text(lines_[i].substr(0, end_byte), font_).right - scroll_offset_x;
                mem_gr.draw_rect({ x1, actual_y, x2, actual_y + line_height }, theme_color(tcn, tv_selection, theme_));
            }
            mem_gr.draw_text({ 0 - scroll_offset_x, actual_y}, lines_[i], theme_color(tcn, tv_text, theme_), font_);
            
            // Cursor
            if (cursor_visible && i == cursor_row)
            {
                size_t max_col = utf8::distance(lines_[i].begin(), lines_[i].end());
                size_t safe_cursor_col = std::min(cursor_col, max_col);
                size_t cursor_byte = get_byte_pos_for_char_pos(lines_[i], safe_cursor_col);
                int cursor_x = mem_gr.measure_text(lines_[i].substr(0, cursor_byte), font_).right - scroll_offset_x;
                mem_gr.draw_line({ cursor_x, actual_y, cursor_x, actual_y + line_height }, theme_color(tcn, tv_cursor, theme_));
            }
        }
        // Copying the offscreen buffer to the parent context
        gr.draw_graphic({ control_pos.left + border_width + INPUT_HORIZONTAL_INDENT,
            control_pos.top + border_width,
            control_pos.left + content_width - ((border_width + INPUT_HORIZONTAL_INDENT) * 2),
            control_pos.top + content_height - (border_width * 2) },
            mem_gr, 0, 0);
        
        // Rendering scrollbars
        if (vert_scroll->showed())
        {
            vert_scroll->draw(gr, {});
        }
        if (hor_scroll->showed())
        {
            hor_scroll->draw(gr, {});
        }
    }
    else
    {
        /// Create memory dc for text and selection bar
        auto full_text_width = get_text_width(gr, text_, text_.size(), font_) + 2;

        auto parent__ = parent_.lock(); if (!parent__) return;
        system_context ctx = parent__->context();
        graphic mem_gr(ctx);
        
        mem_gr.init({ 0, 0,
            full_text_width,
            position_.height() },
            theme_color(tcn, tv_background, theme_));

        /// Draw the selection bar
        if (select_start_position != select_end_position)
        {
            auto start_coordinate = get_text_width(mem_gr, text_, select_start_position, font_);
            auto end_coordinate = get_text_width(mem_gr, text_, select_end_position, font_);

            mem_gr.draw_rect({ start_coordinate, 0, end_coordinate, line_height }, theme_color(tcn, tv_selection, theme_));
        }

        /// Draw the text
        if (input_view_ != input_view::password)
        {
            mem_gr.draw_text({ 0 }, text_, theme_color(tcn, tv_text, theme_), font_);
        }
        else
        {
            std::string text__;
            for (auto i = 0; i != text_.size(); ++i)
            {
                if (check_count_valid(i))
                {
                    text__.append("*");
                }
            }
            mem_gr.draw_text({ 0 }, text__, theme_color(tcn, tv_text, theme_), font_);
        }
            
        /// Draw the cursor
        if (cursor_visible)
        {
            auto cursor_coordinate = get_text_width(mem_gr, text_, cursor_position, font_);
            mem_gr.draw_line({ cursor_coordinate, 0, cursor_coordinate, line_height }, theme_color(tcn, tv_cursor, theme_));

            while (cursor_coordinate - left_shift >= position_.width() - INPUT_HORIZONTAL_INDENT * 2)
            {
                left_shift += 10;
            }

            while (left_shift > cursor_coordinate)
            {
                left_shift -= 10;
            }
        }

        int32_t input_vertical_indent = position_.height() > line_height ? (position_.height() - line_height) / 2 : 0;
    
        gr.draw_graphic({ control_pos.left + border_width + INPUT_HORIZONTAL_INDENT,
            control_pos.top + input_vertical_indent,
            control_pos.width() - ((INPUT_HORIZONTAL_INDENT + border_width) * 2),
            control_pos.height() - input_vertical_indent * 2 },
            mem_gr, left_shift, 0);
    }
}

size_t input::calculate_mouse_cursor_position(int32_t x)
{
    if (text_.empty())
    {
        return 0;
    }

    x -= position().left + INPUT_HORIZONTAL_INDENT - left_shift;

    auto parent__ = parent_.lock(); if (!parent__) return 0;
    system_context ctx = parent__->context(); graphic mem_gr(ctx);
    mem_gr.init({ 0, 0, position_.width(), position_.height() }, { 0 } );

    auto font_ = theme_font(tcn, tv_font, theme_);
    if (input_view_ == input_view::password)
    {
#ifdef _WIN32
        font_.name = "Courier New";
#elif __linux__
        font_.name = "monospace";
#endif
    }

    int32_t text_width = 0;
    size_t count = 0;
    while (x > text_width && count != text_.size())
    {
        ++count;

        if (check_count_valid(count))
        {
            text_width = get_text_width(mem_gr, text_, count, font_);
        }
    }

    return count;
}

void input::update_select_positions(bool shift_pressed, size_t start_position, size_t end_position)
{
    if (shift_pressed)
    {
        if (!selecting)
        {
            selecting = true;
            select_start_position = start_position;
            select_end_position = end_position;
        }
        else
        {
            select_end_position = end_position;
        }
    }
    else
    {
        selecting = false;
        select_start_position = 0;
        select_end_position = 0;
    }
}

bool input::clear_selected_text()
{
    if (select_start_position != select_end_position)
    {
        size_t start = 0, end = 0;

        if (select_end_position > select_start_position)
        {
            start = select_start_position;
            end = select_end_position;
        }
        else
        {
            start = select_end_position;
            end = select_start_position;
        }

        cursor_position = start;

        text_.erase(start, end - start);

        selecting = false;
        select_start_position = 0;
        select_end_position = 0;

        return true;
    }

    return false;
}

void input::select_current_word(int32_t x)
{
    cursor_position = calculate_mouse_cursor_position(x);

    select_start_position = cursor_position;
    select_end_position = cursor_position;

    while (select_start_position != 0 && text_[select_start_position] != ' ')
    {
        --select_start_position;
    }

    if (text_[select_start_position] == ' ') // remove first space from selection
    {
        ++select_start_position;
    }

    while (select_end_position != text_.size() && text_[select_end_position] != ' ')
    {
        ++select_end_position;
    }

    redraw();
    scroll_to_cursor();
}

void input::select_all()
{
    select_start_position = 0;
    select_end_position = text_.size();;

    redraw();
    scroll_to_cursor();
}

bool input::check_count_valid(size_t count)
{
    if (count == 0 || text_.empty())
        return true;

    auto end_it = utf8::find_invalid(text_.begin(), text_.begin() + count);
    return end_it == text_.begin() + count;
}

void input::move_cursor_left()
{
    --cursor_position;
    while (!check_count_valid(cursor_position))
    {
        --cursor_position;
    }
}

void input::move_cursor_right()
{
    ++cursor_position;
    while (!check_count_valid(cursor_position))
    {
        ++cursor_position;
    }
}

bool is_number(std::string_view s)
{
    return s.find_first_not_of("-,.0123456789") != std::string::npos;
}

// Auxiliary function for multiline
std::pair<size_t, size_t> input::calculate_mouse_cursor_position_multiline(int x, int y)
{
    auto parent__ = parent_.lock(); if (!parent__) return { 0, 0 };
    system_context ctx = parent__->context();
    graphic mem_gr(ctx);
    mem_gr.init({ 0, 0, position_.width(), position_.height() }, theme_color(tcn, tv_background, theme_));

    auto control_pos = position();
    auto border_width = theme_dimension(tcn, tv_border_width, theme_);
    auto font_ = theme_font(tcn, tv_font, theme_);
    int line_height = font_.size;
    
    // We take into account scrolling and borders
    int rel_y = y - control_pos.top - border_width + scroll_offset_y;
    size_t row = std::min((size_t)(rel_y / line_height), lines_.size() - 1);
    
    int rel_x = x - control_pos.left - border_width - INPUT_HORIZONTAL_INDENT + scroll_offset_x;
    
    // We use character positions to measure text
    size_t char_count = utf8::distance(lines_[row].begin(), lines_[row].end());
    size_t col = 0;
    
    for (; col <= char_count; ++col)
    {
        size_t byte_pos = get_byte_pos_for_char_pos(lines_[row], col);
        int w = mem_gr.measure_text(lines_[row].substr(0, byte_pos), font_).right;
        if (w > rel_x) break;
    }
    
    if (col > 0) {
        --col;
    }
    if (col >= char_count) {
        col = char_count;
    }

    return {row, col};
}

void input::receive_control_events(const event &ev)
{
    if (!showed_ || !enabled_)
    {
        return;
    }

    // Scrollbar event handling for multiline
    if (input_view_ == input_view::multiline) {
        if (ev.type == event_type::mouse && (ev.mouse_event_.type == mouse_event_type::move || ev.mouse_event_.type == mouse_event_type::enter)) {
            auto parent__ = parent_.lock();
            if (parent__) {
                if (vert_scroll->position().in(ev.mouse_event_.x, ev.mouse_event_.y) || hor_scroll->position().in(ev.mouse_event_.x, ev.mouse_event_.y)) {
                    set_cursor(parent__->context(), cursor::default_);
                } else {
                    set_cursor(parent__->context(), cursor::ibeam);
                }
            }
            // Enter/leave emulation for scrollbars
            bool vert_hover = vert_scroll->position().in(ev.mouse_event_.x, ev.mouse_event_.y);
            bool hor_hover = hor_scroll->position().in(ev.mouse_event_.x, ev.mouse_event_.y);
            static bool prev_vert_hover = false;
            static bool prev_hor_hover = false;
            if (vert_hover && !prev_vert_hover) {
                event enter_ev = ev;
                enter_ev.mouse_event_.type = mouse_event_type::enter;
                vert_scroll->receive_control_events(enter_ev);
            } else if (!vert_hover && prev_vert_hover) {
                event leave_ev = ev;
                leave_ev.mouse_event_.type = mouse_event_type::leave;
                vert_scroll->receive_control_events(leave_ev);
            }
            if (hor_hover && !prev_hor_hover) {
                event enter_ev = ev;
                enter_ev.mouse_event_.type = mouse_event_type::enter;
                hor_scroll->receive_control_events(enter_ev);
            } else if (!hor_hover && prev_hor_hover) {
                event leave_ev = ev;
                leave_ev.mouse_event_.type = mouse_event_type::leave;
                hor_scroll->receive_control_events(leave_ev);
            }
            prev_vert_hover = vert_hover;
            prev_hor_hover = hor_hover;
        }
        // Checking whether the cursor is above the scrollbars for mouse events
        if (vert_scroll->position().in(ev.mouse_event_.x, ev.mouse_event_.y)) {
            vert_scroll->receive_control_events(ev);
            return;
        }
        if (hor_scroll->position().in(ev.mouse_event_.x, ev.mouse_event_.y)) {
            hor_scroll->receive_control_events(ev);
            return;
        }
        if (ev.type == event_type::mouse && ev.mouse_event_.type == mouse_event_type::left_down) {
            auto [row, col] = calculate_mouse_cursor_position_multiline(ev.mouse_event_.x, ev.mouse_event_.y);
            cursor_row = row;
            cursor_col = col;
            selecting = true;
            select_start_row = cursor_row;
            select_start_col = cursor_col;
            select_end_row = cursor_row;
            select_end_col = cursor_col;
            redraw();
            scroll_to_cursor();
        }
    }

    if (ev.type == event_type::mouse)
    {
        switch (ev.mouse_event_.type)
        {
            case mouse_event_type::enter:
            {
                auto parent__ = parent_.lock();
                if (parent__)
                {
                    set_cursor(parent__->context(), cursor::ibeam);
                }
                stop_auto_scroll();
            }
            break;
            case mouse_event_type::leave:
            {
                if (selecting)
                {
                    if (select_start_position < select_end_position)
                    {
                        select_end_position = text_.size();
                        cursor_position = select_end_position;
                    }
                    else
                    {
                        select_end_position = 0;
                        cursor_position = 0;
                    }

                    auto control_pos = position();

                    // Если мышь выше видимой области, запускаем автоматическую прокрутку вверх
                    if (ev.mouse_event_.y < control_pos.top && cursor_row > 0) {
                        start_auto_scroll(true);
                        return;
                    }

                    // Если мышь ниже видимой области, запускаем автоматическую прокрутку вниз
                    if (ev.mouse_event_.y > control_pos.bottom && cursor_row + 1 < lines_.size()) {
                        start_auto_scroll(false);
                        return;
                    }
                }

                auto parent__ = parent_.lock();
                if (parent__)
                {
                    set_cursor(parent__->context(), cursor::default_);
                }
            }
            break;
            case mouse_event_type::left_down:
                if (input_view_ == input_view::multiline) {
                    auto [row, col] = calculate_mouse_cursor_position_multiline(ev.mouse_event_.x, ev.mouse_event_.y);
                    cursor_row = row;
                    cursor_col = col;
                    selecting = true;
                    select_start_row = cursor_row;
                    select_start_col = cursor_col;
                    select_end_row = cursor_row;
                    select_end_col = cursor_col;
                    redraw();
                    scroll_to_cursor();
                } else {
                cursor_position = calculate_mouse_cursor_position(ev.mouse_event_.x);
                selecting = true;
                select_start_position = cursor_position;
                select_end_position = select_start_position;
                redraw();
                    scroll_to_cursor();
                }
            break;
            case mouse_event_type::left_up:
                selecting = false;
                stop_auto_scroll();
                menu_->hide();
            break;
            case mouse_event_type::right_up:
                if (input_view_ == input_view::multiline) {
                    bool has_selection = !(select_start_row == select_end_row && select_start_col == select_end_col);
                    menu_->update_item({ 0, has_selection && input_view_ != input_view::readonly && input_view_ != input_view::password ? menu_item_state::normal : menu_item_state::disabled,
                        locale(tc, cl_cut).data(), "Ctrl+X", nullptr, {}, [this](int32_t i) { buffer_cut_multiline(); parent_.lock()->set_focused(shared_from_this()); } });
                    menu_->update_item({ 1, has_selection && input_view_ != input_view::password ? menu_item_state::normal : menu_item_state::disabled,
                        locale(tc, cl_copy).data(), "Ctrl+C", nullptr, {}, [this](int32_t i) { buffer_copy_multiline(); parent_.lock()->set_focused(shared_from_this()); } });
                    menu_->update_item({ 2, input_view_ != input_view::readonly ? menu_item_state::normal : menu_item_state::disabled,
                        locale(tc, cl_paste).data(), "Ctrl+V", nullptr, {}, [this](int32_t i) { buffer_paste_multiline(); parent_.lock()->set_focused(shared_from_this()); } });
                } else {
                menu_->update_item({ 0, select_start_position != select_end_position && input_view_ != input_view::readonly && input_view_ != input_view::password ? menu_item_state::normal : menu_item_state::disabled,
                    locale(tc, cl_cut).data(), "Ctrl+X", nullptr, {}, [this](int32_t i) { buffer_cut(); parent_.lock()->set_focused(shared_from_this()); } });
                menu_->update_item({ 1, select_start_position != select_end_position && input_view_ != input_view::password ? menu_item_state::normal : menu_item_state::disabled,
                    locale(tc, cl_copy).data(), "Ctrl+C", nullptr, {}, [this](int32_t i) { buffer_copy(); parent_.lock()->set_focused(shared_from_this()); } });
                menu_->update_item({ 2, input_view_ != input_view::readonly ? menu_item_state::normal : menu_item_state::disabled,
                    locale(tc, cl_paste).data(), "Ctrl+V", nullptr, {}, [this](int32_t i) { buffer_paste(); parent_.lock()->set_focused(shared_from_this()); } });
                }

                menu_->show_on_control(shared_from_this(), 0, ev.mouse_event_.x, ev.mouse_event_.y);
            break;
            case mouse_event_type::move:
                if (selecting && input_view_ == input_view::multiline) {
                    // Обычная обработка для видимой области
                    auto [row, col] = calculate_mouse_cursor_position_multiline(ev.mouse_event_.x, ev.mouse_event_.y);
                    cursor_row = row;
                    cursor_col = col;
                    select_end_row = row;
                    select_end_col = col;
                    redraw();
                    scroll_to_cursor();
                } else if (selecting) {
                    auto measured_cursor_position = calculate_mouse_cursor_position(ev.mouse_event_.x);
                    if (cursor_position != measured_cursor_position)
                    {
                        cursor_position = measured_cursor_position;
                        select_end_position = cursor_position;
                        redraw();
                        scroll_to_cursor();
                    }
                }
            break;
            case mouse_event_type::left_double:
                if (input_view_ == input_view::multiline) {
                    select_current_word_multiline(ev.mouse_event_.x, ev.mouse_event_.y);
                } else {
                select_current_word(ev.mouse_event_.x);
                }
            break;
            case mouse_event_type::wheel:
                if (input_view_ == input_view::multiline) {
                    if (ev.mouse_event_.wheel_delta > 0) {
                        vert_scroll->scroll_up();
                    } else {
                        vert_scroll->scroll_down();
                    }
                }
            break;
            default: break;
        }
    }
    else if (ev.type == event_type::keyboard)
    {
        switch (ev.keyboard_event_.type)
        {
            case keyboard_event_type::down:
                timer_.stop();
                cursor_visible = true;
                if (input_view_ == input_view::multiline) {
                    bool shift = (ev.keyboard_event_.modifier == vk_lshift || ev.keyboard_event_.modifier == vk_rshift);
                    switch (ev.keyboard_event_.key[0]) {
                        case vk_left:
                            if (shift) {
                                size_t old_row = cursor_row, old_col = cursor_col;
                                if (cursor_col > 0) {
                                    --cursor_col;
                                } else if (cursor_row > 0) {
                                    --cursor_row;
                                    cursor_col = utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end());
                                }
                                select_end_row = cursor_row; select_end_col = cursor_col;
                                if (!selecting) { select_start_row = old_row; select_start_col = old_col; selecting = true; }
                            } else {
                                if (cursor_col > 0) {
                                    --cursor_col;
                                } else if (cursor_row > 0) {
                                    --cursor_row;
                                    cursor_col = utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end());
                                }
                                selecting = false; select_start_row = select_start_col = select_end_row = select_end_col = 0;
                            }
                            redraw();
                            scroll_to_cursor();
                            break;
                        case vk_right:
                            if (shift) {
                                size_t old_row = cursor_row, old_col = cursor_col;
                                size_t max_col = utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end());
                                if (cursor_col < max_col) {
                                    ++cursor_col;
                                } else if (cursor_row + 1 < lines_.size()) {
                                    ++cursor_row;
                                    cursor_col = 0;
                                }
                                select_end_row = cursor_row; select_end_col = cursor_col;
                                if (!selecting) { select_start_row = old_row; select_start_col = old_col; selecting = true; }
                            } else {
                                size_t max_col = utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end());
                                if (cursor_col < max_col) {
                                    ++cursor_col;
                                } else if (cursor_row + 1 < lines_.size()) {
                                    ++cursor_row;
                                    cursor_col = 0;
                                }
                                selecting = false; select_start_row = select_start_col = select_end_row = select_end_col = 0;
                            }
                            redraw();
                            scroll_to_cursor();
                            break;
                        case vk_up:
                            if (shift) {
                                size_t old_row = cursor_row, old_col = cursor_col;
                                if (cursor_row > 0) { --cursor_row; cursor_col = std::min(cursor_col, static_cast<size_t>(utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end()))); }
                                select_end_row = cursor_row; select_end_col = cursor_col;
                                if (!selecting) { select_start_row = old_row; select_start_col = old_col; selecting = true; }
                            } else {
                                if (cursor_row > 0) { --cursor_row; cursor_col = std::min(cursor_col, static_cast<size_t>(utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end()))); }
                                selecting = false; select_start_row = select_start_col = select_end_row = select_end_col = 0;
                            }
                            redraw();
                            scroll_to_cursor();
                            break;
                        case vk_down:
                            if (shift) {
                                size_t old_row = cursor_row, old_col = cursor_col;
                                if (cursor_row + 1 < lines_.size()) { 
                                    ++cursor_row; 
                                    cursor_col = std::min(cursor_col, static_cast<size_t>(utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end()))); 
                                } else {
                                    // Если уже на последней строке, двигаем курсор в конец строки
                                    cursor_col = utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end());
                                }
                                select_end_row = cursor_row; select_end_col = cursor_col;
                                if (!selecting) { select_start_row = old_row; select_start_col = old_col; selecting = true; }
                            } else {
                                if (cursor_row + 1 < lines_.size()) { ++cursor_row; cursor_col = std::min(cursor_col, static_cast<size_t>(utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end()))); }
                                selecting = false; select_start_row = select_start_col = select_end_row = select_end_col = 0;
                            }
                            redraw();
                            scroll_to_cursor();
                            break;
                        case vk_home:
                            if (shift) {
                                size_t old_row = cursor_row, old_col = cursor_col;
                                cursor_col = 0;
                                select_end_row = cursor_row; select_end_col = cursor_col;
                                if (!selecting) { select_start_row = old_row; select_start_col = old_col; selecting = true; }
                            } else {
                                cursor_col = 0;
                                selecting = false; select_start_row = select_start_col = select_end_row = select_end_col = 0;
                            }
                            redraw();
                            scroll_to_cursor();
                            break;
                        case vk_end:
                            if (shift) {
                                size_t old_row = cursor_row, old_col = cursor_col;
                                // We use a symbolic position for the selection to work correctly
                                cursor_col = utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end());
                                select_end_row = cursor_row; select_end_col = cursor_col;
                                if (!selecting) { select_start_row = old_row; select_start_col = old_col; selecting = true; }
                            } else {
                                cursor_col = utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end());
                                selecting = false; select_start_row = select_start_col = select_end_row = select_end_col = 0;
                            }
                            redraw();
                            scroll_to_cursor();
                            break;
                        case vk_back:
                            if (clear_selected_text_multiline()) {
                                update_scroll_areas();
                                scroll_to_cursor();
                                redraw();
                                if (change_callback) change_callback(text_multiline());
                                break;
                            }
                            if (cursor_col > 0) {
                                auto prev_position = cursor_col;
                                --cursor_col;
                                // We get byte positions for correct deletion of UTF-8 characters
                                size_t prev_byte = get_byte_pos_for_char_pos(lines_[cursor_row], prev_position);
                                size_t curr_byte = get_byte_pos_for_char_pos(lines_[cursor_row], cursor_col);
                                lines_[cursor_row].erase(curr_byte, prev_byte - curr_byte);
                            } else if (cursor_row > 0) {
                                --cursor_row;
                                cursor_col = utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end());
                                lines_[cursor_row] += lines_[cursor_row + 1];
                                lines_.erase(lines_.begin() + cursor_row + 1);
                            }
                            invalidate_max_width_cache();
                            update_scroll_areas();
                            scroll_to_cursor();
                            redraw();
                            if (change_callback) change_callback(text_multiline());
                            break;
                        case vk_del:
                        {
                            if (clear_selected_text_multiline()) {
                                update_scroll_areas();
                                scroll_to_cursor();
                                redraw();
                                if (change_callback) change_callback(text_multiline());
                                break;
                            }
                            if (cursor_col < lines_[cursor_row].size()) {
                                size_t char_count = 1;
                                while (!check_count_valid_multiline(lines_[cursor_row], cursor_col + char_count) && cursor_col + char_count < lines_[cursor_row].size()) {
                                    ++char_count;
                                }
                                // We get byte positions for correct deletion of UTF-8 characters
                                size_t start_byte = get_byte_pos_for_char_pos(lines_[cursor_row], cursor_col);
                                size_t end_byte = get_byte_pos_for_char_pos(lines_[cursor_row], cursor_col + char_count);
                                lines_[cursor_row].erase(start_byte, end_byte - start_byte);
                            } else if (cursor_row + 1 < lines_.size()) {
                                lines_[cursor_row] += lines_[cursor_row + 1];
                                lines_.erase(lines_.begin() + cursor_row + 1);
                            }
                            invalidate_max_width_cache();
                            update_scroll_areas();
                            scroll_to_cursor();
                            redraw();
                            if (change_callback) change_callback(text_multiline());
                        }
                            break;
                        case vk_return: case vk_rreturn: {
                            // We get byte positions for correct operation with UTF-8
                            size_t cursor_byte = get_byte_pos_for_char_pos(lines_[cursor_row], cursor_col);
                            std::string new_line = lines_[cursor_row].substr(cursor_byte);
                            lines_[cursor_row].erase(cursor_byte);
                            lines_.insert(lines_.begin() + cursor_row + 1, new_line);
                            ++cursor_row;
                            cursor_col = 0;
                            invalidate_max_width_cache();
                            update_scroll_areas();
                            scroll_to_cursor();
                            redraw();
                            if (change_callback) change_callback(text_multiline());
                            break;
                        }
                        case vk_page_up: case vk_npage_up:
                            if (cursor_row > 0) {
                                auto border_width = theme_dimension(tcn, tv_border_width, theme_);
                                auto font_ = theme_font(tcn, tv_font, theme_);
                                int line_height = font_.size;
                                
                                int content_height = position().height() - border_width * 2 - SCROLL_SIZE;

                                int visible_lines = std::max(1, content_height / line_height);
                                size_t new_row = cursor_row > (size_t)visible_lines ? cursor_row - visible_lines : 0;
                                cursor_row = new_row;
                                cursor_col = std::min(cursor_col, static_cast<size_t>(utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end())));
                                selecting = false; select_start_row = select_start_col = select_end_row = select_end_col = 0;
                                redraw();
                                scroll_to_cursor();
                            }
                            break;
                        case vk_page_down: case vk_npage_down:
                            if (cursor_row + 1 < lines_.size()) {
                                auto border_width = theme_dimension(tcn, tv_border_width, theme_);
                                auto font_ = theme_font(tcn, tv_font, theme_);
                                int line_height = font_.size;
                                int content_height = position().height() - border_width * 2 - SCROLL_SIZE;
                                int visible_lines = std::max(1, content_height / line_height);
                                size_t new_row = std::min(cursor_row + visible_lines, lines_.size() - 1);
                                cursor_row = new_row;
                                cursor_col = std::min(cursor_col, static_cast<size_t>(utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end())));
                                selecting = false; select_start_row = select_start_col = select_end_row = select_end_col = 0;
                                redraw();
                                scroll_to_cursor();
                            }
                            break;
                        default:
                            break;
                    }
                } else {
                switch (ev.keyboard_event_.key[0])
                {
                    case vk_left: case vk_nleft:
                        if (ev.keyboard_event_.key[0] == vk_nleft && ev.keyboard_event_.modifier == vk_numlock)
                        {
                            return;
                        }
                        if (cursor_position > 0)
                        {
                            auto prev_position = cursor_position;

                            move_cursor_left();

                            update_select_positions(ev.keyboard_event_.modifier == vk_lshift ||
                                ev.keyboard_event_.modifier == vk_rshift,
                                prev_position,
                                cursor_position);
                            redraw();
                                scroll_to_cursor();
                        }
                    break;
                    case vk_right: case vk_nright:
                        if (ev.keyboard_event_.key[0] == vk_nright && ev.keyboard_event_.modifier == vk_numlock)
                        {
                            return;
                        }
                        if (cursor_position < text_.size())
                        {
                            auto prev_position = cursor_position;

                            move_cursor_right();

                            update_select_positions(ev.keyboard_event_.modifier == vk_lshift ||
                                ev.keyboard_event_.modifier == vk_rshift,
                                prev_position,
                                cursor_position);
                            redraw();
                                scroll_to_cursor();
                        }
                    break;
                    case vk_home: case vk_nhome:
                        if (ev.keyboard_event_.key[0] == vk_nhome && ev.keyboard_event_.modifier == vk_numlock)
                        {
                            return;
                        }
                        update_select_positions(ev.keyboard_event_.modifier == vk_lshift ||
                            ev.keyboard_event_.modifier == vk_rshift,
                            cursor_position, 0);
                        cursor_position = 0;
                        redraw();
                            scroll_to_cursor();
                    break;
                    case vk_end: case vk_nend:
                        if (ev.keyboard_event_.key[0] == vk_nend && ev.keyboard_event_.modifier == vk_numlock)
                        {
                            return;
                        }
                        if (!text_.empty())
                        {
                            update_select_positions(ev.keyboard_event_.modifier == vk_lshift ||
                                ev.keyboard_event_.modifier == vk_rshift,
                                cursor_position,
                                text_.size());

                            cursor_position = text_.size();

                            redraw();
                                scroll_to_cursor();
                        }
                    break;
                    case vk_back:
                        if (input_view_ != input_view::readonly)
                        {
                            if (!clear_selected_text() && cursor_position > 0)
                            {
                                auto prev_position = cursor_position;

                                move_cursor_left();

                                text_.erase(cursor_position, prev_position - cursor_position);
                            }
                            
                            redraw();

                            if (change_callback)
                            {
                                change_callback(text_);
                            }
                        }
                    break;
                    case vk_del:
                        if (input_view_ != input_view::readonly && !text_.empty())
                        {
                            if (!clear_selected_text())
                            {
                                if (text_.size() == cursor_position)
                                {
                                    return;
                                }

                                size_t char_count = 1;
                                while (!check_count_valid(cursor_position + char_count))
                                {
                                    ++char_count;
                                }
                                text_.erase(cursor_position, char_count);
                            }
                            
                            redraw();

                            if (change_callback)
                            {
                                change_callback(text_);
                            }
                        }
                    break;
                    case vk_return: case vk_rreturn:
                        if (return_callback)
                        {
                            return_callback();
                        }
                    break;
                    }
                }
            break;
            case keyboard_event_type::up:
                timer_.start(500);

                if (ev.keyboard_event_.key[0] == vk_lshift || ev.keyboard_event_.key[0] == vk_rshift)
                {
                    selecting = false;
                }
            break;
            case keyboard_event_type::key:
                if (ev.keyboard_event_.key[0] == 0x3)       // ctrl + c
                {
                    if (input_view_ == input_view::multiline) {
                        return buffer_copy_multiline();
                    } else {
                    return buffer_copy();
                    }
                }
                else if (ev.keyboard_event_.key[0] == 0x18) // ctrl + x
                {
                    if (input_view_ == input_view::multiline) {
                        return buffer_cut_multiline();
                    } else {
                    return buffer_cut();
                    }
                }
                else if (ev.keyboard_event_.key[0] == 0x16) // ctrl + v
                {
                    if (input_view_ != input_view::readonly) {
                        if (input_view_ == input_view::multiline) {
                            return buffer_paste_multiline();
                        } else {
                        return buffer_paste();
                        }
                    }
                }
                else if (ev.keyboard_event_.key[0] == 0x1)  // ctrl + a
                {
                    if (input_view_ == input_view::multiline) {
                        return select_all_multiline();
                    } else {
                    return select_all();
                    }
                }
                else if (ev.keyboard_event_.key[0] == 0x7f) // ctrl + backspace
                {
                    if (input_view_ != input_view::readonly)
                    {
                        return set_text("");
                    }
                }

                if (input_view_ == input_view::readonly ||
                    ev.keyboard_event_.key[0] == vk_tab ||
                    text_.size() >= symbols_limit)
                {
                    return;
                }

                if (input_content_ == input_content::integer &&
                    !std::isdigit(ev.keyboard_event_.key[0]))
                {
                    return;
                }

                if (input_content_ == input_content::numeric &&
                    !is_number(ev.keyboard_event_.key))
                {
                    return;
                }

                if (input_view_ == input_view::multiline)
                {
                    if (clear_selected_text_multiline())
                    {
                        redraw();
                        if (change_callback)
                        {
                            change_callback(text_multiline());
                        }
                    }
                    if (lines_[cursor_row].size() < (size_t)symbols_limit)
                    {
                        size_t insert_byte = get_byte_pos_for_char_pos(lines_[cursor_row], cursor_col);
                        lines_[cursor_row].insert(insert_byte, ev.keyboard_event_.key, ev.keyboard_event_.key_size);
                        cursor_col += utf8::distance(ev.keyboard_event_.key, ev.keyboard_event_.key + ev.keyboard_event_.key_size);
                        invalidate_max_width_cache();
                        update_scroll_areas();
                        scroll_to_cursor();
                        redraw();
                        if (change_callback)
                        {
                            change_callback(text_multiline());
                        }
                    }
                    return;
                }
                
                clear_selected_text();

                text_.insert(cursor_position, ev.keyboard_event_.key);
                
                cursor_position += ev.keyboard_event_.key_size;

                redraw();
                scroll_to_cursor();

                if (change_callback)
                {
                    change_callback(text_);
                }
            break;
        }
    }
    else if (ev.type == event_type::internal)
    {
        switch (ev.internal_event_.type)
        {
            case internal_event_type::set_focus:
                focused_ = true;

                cursor_position = text_.size();

                redraw();

                timer_.start(500);
            break;
            case internal_event_type::remove_focus:
                focused_ = false;

                cursor_visible = false;

                selecting = false;
                select_start_position = select_end_position = 0;

                timer_.stop();

                redraw();
            break;
        }
    }
}

void input::receive_plain_events(const event &ev)
{
    if (ev.type == event_type::mouse && ev.mouse_event_.type == mouse_event_type::left_up)
    {
        selecting = false;
    }
}

void input::set_position(rect position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent_);
    if (input_view_ == input_view::multiline) {
        auto border_width = theme_dimension(tcn, tv_border_width, theme_);
        vert_scroll->set_position({ position_.right - 14 - border_width,
            position_.top + border_width,
            position_.right - border_width,
            position_.bottom - border_width });
        hor_scroll->set_position({ position_.left + border_width,
            position_.bottom - 14 - border_width,
            position_.right - border_width,
            position_.bottom - border_width });
        update_scroll_areas();
    }
}

rect input::position() const
{
    return get_control_position(position_, parent_);
}

void input::set_parent(std::shared_ptr<window> window_)
{
    parent_ = window_;
    my_control_sid = window_->subscribe(std::bind(&input::receive_control_events, this, std::placeholders::_1),
        wui::flags_map<wui::event_type>(3, wui::event_type::internal, wui::event_type::mouse, wui::event_type::keyboard),
        shared_from_this());
    my_plain_sid = window_->subscribe(std::bind(&input::receive_plain_events, this, std::placeholders::_1), event_type::mouse);
    window_->add_control(menu_, { 0 });
    if (input_view_ == input_view::multiline) {
        window_->add_control(vert_scroll, { 0 });
        window_->add_control(hor_scroll, { 0 });
    }
}

std::weak_ptr<window> input::parent() const
{
    return parent_;
}

void input::clear_parent()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(menu_);
        if (input_view_ == input_view::multiline) {
            parent__->remove_control(vert_scroll);
            parent__->remove_control(hor_scroll);
        }
        parent__->unsubscribe(my_control_sid);
        parent__->unsubscribe(my_plain_sid);
    }
    parent_.reset();
}

void input::set_topmost(bool yes)
{
    topmost_ = yes;
}

bool input::topmost() const
{
    return topmost_;
}

bool input::focused() const
{
    return focused_;
}

bool input::focusing() const
{
    return enabled_ && showed_;
}

error input::get_error() const
{
    return {};
}

void input::update_theme_control_name(std::string_view theme_control_name)
{
    tcn = theme_control_name;
    update_theme(theme_);
}

void input::update_theme(std::shared_ptr<i_theme> theme__)
{
    theme_ = theme__;
}

void input::show()
{
    showed_ = true;
    redraw();
}

void input::hide()
{
    showed_ = false;
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->redraw(position(), true);
    }
}

bool input::showed() const
{
    return showed_;
}

void input::enable()
{
    enabled_ = true;
    redraw();
}

void input::disable()
{
    enabled_ = false;
    redraw();
}

bool input::enabled() const
{
    return enabled_;
}

void input::set_text(std::string_view text__)
{
    if (input_view_ == input_view::multiline) {
        set_text_multiline(text__);
        reset_multiline_state();
    } else {
    text_ = text__;
    cursor_position = 0;
    }
    redraw();
    if (change_callback) {
        if (input_view_ == input_view::multiline)
            change_callback(text_multiline());
        else
        change_callback(text_);
    }
}

std::string input::text() const
{
    if (input_view_ == input_view::multiline)
        return text_multiline();
    else
    return text_;
}

void input::set_input_view(input_view input_view__)
{
    input_view_ = input_view__;
    if (input_view_ == input_view::multiline) {
        set_text_multiline(text_);
        reset_multiline_state();
    } else {
        text_ = text_multiline();
        cursor_position = 0;
    }
}

void input::set_input_content(input_content input_content__)
{
    input_content_ = input_content__;
}

void input::set_symbols_limit(int32_t symbols_limit_)
{
    symbols_limit = symbols_limit_;
}

void input::set_change_callback(std::function<void(const std::string&)> change_callback_)
{
    change_callback = change_callback_;
}

void input::set_return_callback(std::function<void()> return_callback_)
{
    return_callback = return_callback_;
}

void input::redraw()
{
    if (showed_)
    {
        auto parent__ = parent_.lock();
        if (parent__)
        {
            parent__->redraw(position());
        }
    }
}

void input::redraw_cursor()
{
    cursor_visible = !cursor_visible;
    redraw();
}

void input::buffer_copy()
{
    if (!parent_.lock() || select_start_position == select_end_position || input_view_ == input_view::password)
    {
        return;
    }

    size_t start = 0, end = 0;

    if (select_end_position > select_start_position)
    {
        start = select_start_position;
        end = select_end_position;
    }
    else
    {
        start = select_end_position;
        end = select_start_position;
    }

    clipboard_put(text_.substr(start, end - start), parent_.lock()->context());
}

void input::buffer_cut()
{
    if (select_start_position == select_end_position || input_view_ == input_view::readonly)
    {
        return;
    }

    buffer_copy();

    clear_selected_text();

    redraw();

    if (change_callback)
    {
        change_callback(text_);
    }
}

void input::buffer_paste()
{
    if (!parent_.lock() || input_view_ == input_view::readonly || !is_text_in_clipboard(parent_.lock()->context()))
    {
        return;
    }

    clear_selected_text();

    auto paste_string = clipboard_get_text(parent_.lock()->context());

    if (paste_string.size() + text_.size() > symbols_limit)
    {
        auto need_to_erase = (paste_string.size() + text_.size()) - symbols_limit;
        if (paste_string.size() > need_to_erase)
        {
            paste_string.resize(paste_string.size() - need_to_erase);
        }
        else
            return;
    }
    
    text_.insert(cursor_position, paste_string);

    cursor_position += paste_string.size();

    redraw();
    scroll_to_cursor();

    if (change_callback)
    {
        change_callback(text_);
    }
}

void input::set_text_multiline(std::string_view text) {
    lines_.clear();
    if (text.empty()) {
        lines_.push_back("");
        cursor_row = 0;
        cursor_col = 0;
        invalidate_max_width_cache();
        return;
    }
    std::istringstream iss;
    iss.str(text.data());
    std::string line;
    while (std::getline(iss, line)) {
        lines_.push_back(line);
    }
    if (lines_.empty()) {
        lines_.push_back("");
    }
    cursor_row = 0;
    cursor_col = 0;
    invalidate_max_width_cache();
    update_scroll_areas();
}

std::string input::text_multiline() const
{
    std::ostringstream oss;
    for (size_t i = 0; i < lines_.size(); ++i) {
        oss << lines_[i];
        if (i + 1 < lines_.size()) oss << '\n';
    }
    return oss.str();
}

void input::reset_multiline_state()
{
    cursor_row = cursor_col = 0;
    select_start_row = select_start_col = select_end_row = select_end_col = 0;
}

// Deleting selected text in multiline
bool input::clear_selected_text_multiline()
{
    if (select_start_row == select_end_row && select_start_col == select_end_col)
        return false;
    size_t srow = select_start_row, scol = select_start_col, erow = select_end_row, ecol = select_end_col;
    if (cursor_less(erow, ecol, srow, scol)) std::swap(srow, erow), std::swap(scol, ecol);
    if (srow == erow) {
        // We get byte positions for correct deletion of UTF-8 characters
        size_t start_byte = get_byte_pos_for_char_pos(lines_[srow], scol);
        size_t end_byte = get_byte_pos_for_char_pos(lines_[srow], ecol);
        lines_[srow].erase(start_byte, end_byte - start_byte);
        cursor_row = srow;
        cursor_col = scol;
    } else {
        // The first line
        size_t start_byte = get_byte_pos_for_char_pos(lines_[srow], scol);
        lines_[srow].erase(start_byte);
        
        // The last line
        size_t end_byte = get_byte_pos_for_char_pos(lines_[erow], ecol);
        lines_[erow].erase(0, end_byte);
        
        // Combining the lines
        lines_[srow] += lines_[erow];
        lines_.erase(lines_.begin() + srow + 1, lines_.begin() + erow + 1);
        cursor_row = srow;
        cursor_col = scol;
        invalidate_max_width_cache();
    }
    select_start_row = select_start_col = select_end_row = select_end_col = 0;
    selecting = false;
    return true;
}

// Auxiliary functions for multiline
void input::select_all_multiline() {
    if (lines_.empty()) return;
    select_start_row = 0;
    select_start_col = 0;
    select_end_row = lines_.size() - 1;
    select_end_col = utf8::distance(lines_[select_end_row].begin(), lines_[select_end_row].end());
    redraw();
    scroll_to_cursor();
}

void input::select_current_word_multiline(int x, int y) {
    auto [row, col] = calculate_mouse_cursor_position_multiline(x, y);
    cursor_row = row;
    cursor_col = col;
    
    auto& line = lines_[row];
    select_start_row = select_end_row = row;
    select_start_col = select_end_col = col;
    
    // We are looking for the beginning of a word (using character positions)
    while (select_start_col > 0) {
        auto prev_char_pos = select_start_col - 1;
        auto prev_byte_pos = get_byte_pos_for_char_pos(line, prev_char_pos);
        if (prev_byte_pos < line.size() && line[prev_byte_pos] == ' ') {
            break;
        }
        select_start_col = prev_char_pos;
    }
    
    // Looking for the end of a word (using character positions)
    while (select_end_col < lines_[row].size()) {
        auto next_byte_pos = get_byte_pos_for_char_pos(line, select_end_col);
        if (next_byte_pos < line.size() && line[next_byte_pos] == ' ') {
            break;
        }
        select_end_col++;
    }
    
    selecting = true;
    redraw();
    scroll_to_cursor();
}

// Clipboard functions for multiline
void input::buffer_copy_multiline() {
    if (!parent_.lock() || (select_start_row == select_end_row && select_start_col == select_end_col) || input_view_ == input_view::password) {
        return;
    }

    size_t srow = select_start_row, scol = select_start_col, erow = select_end_row, ecol = select_end_col;
    if (cursor_less(erow, ecol, srow, scol)) std::swap(srow, erow), std::swap(scol, ecol);

    std::ostringstream oss;
    if (srow == erow) {
        // We get byte positions for correct copying of UTF-8 characters
        size_t start_byte = get_byte_pos_for_char_pos(lines_[srow], scol);
        size_t end_byte = get_byte_pos_for_char_pos(lines_[srow], ecol);
        oss << lines_[srow].substr(start_byte, end_byte - start_byte);
    } else {
        // The first line
        size_t start_byte = get_byte_pos_for_char_pos(lines_[srow], scol);
        oss << lines_[srow].substr(start_byte);
        
        // Middle lines
        for (size_t i = srow + 1; i < erow; ++i) {
            oss << '\n' << lines_[i];
        }
        
        // The last line
        if (erow > srow) {
            size_t end_byte = get_byte_pos_for_char_pos(lines_[erow], ecol);
            oss << '\n' << lines_[erow].substr(0, end_byte);
        }
    }

    clipboard_put(oss.str(), parent_.lock()->context());
}

void input::buffer_cut_multiline() {
    if ((select_start_row == select_end_row && select_start_col == select_end_col) || input_view_ == input_view::readonly) {
        return;
    }

    buffer_copy_multiline();
    clear_selected_text_multiline();
    redraw();

    if (change_callback) {
        change_callback(text_multiline());
    }
}

void input::buffer_paste_multiline() {
    if (!parent_.lock() || input_view_ == input_view::readonly || !is_text_in_clipboard(parent_.lock()->context())) {
        return;
    }

    // We check that cursor_row does not exceed the boundaries
    if (lines_.empty()) {
        lines_.push_back("");
        cursor_row = 0;
        cursor_col = 0;
    } else if (cursor_row >= lines_.size()) {
        cursor_row = lines_.size() - 1;
        cursor_col = lines_[cursor_row].size();
    }

    clear_selected_text_multiline();

    auto paste_string = clipboard_get_text(parent_.lock()->context());
    
    // Splitting the inserted text into lines
    std::istringstream iss(paste_string);
    std::vector<std::string> paste_lines;
    std::string line;
    while (std::getline(iss, line)) {
        paste_lines.push_back(line);
    }
    if (paste_lines.empty()) paste_lines.push_back("");

    // Checking the character limit
    size_t total_chars = 0;
    for (const auto& l : lines_) total_chars += utf8::distance(l.begin(), l.end());
    for (const auto& l : paste_lines) total_chars += utf8::distance(l.begin(), l.end());
    
    if (total_chars > (size_t)symbols_limit) {
        return; // We do not insert it if the limit is exceeded.
    }

    if (paste_lines.size() == 1) {
        size_t insert_byte = get_byte_pos_for_char_pos(lines_[cursor_row], cursor_col);
        lines_[cursor_row].insert(insert_byte, paste_lines[0]);
        cursor_col += utf8::distance(paste_lines[0].begin(), paste_lines[0].end());
    } else {
        // We insert several lines
        size_t insert_byte = get_byte_pos_for_char_pos(lines_[cursor_row], cursor_col);
        std::string tail = lines_[cursor_row].substr(insert_byte);
        lines_[cursor_row].erase(insert_byte);
        lines_[cursor_row] += paste_lines[0];
        std::vector<std::string> new_lines;
        new_lines.reserve(paste_lines.size() - 1);
        for (size_t i = 1; i < paste_lines.size() - 1; ++i) {
            new_lines.push_back(paste_lines[i]);
        }
        if (paste_lines.size() > 1) {
            new_lines.push_back(paste_lines.back() + tail);
        }
        lines_.insert(lines_.begin() + cursor_row + 1, new_lines.begin(), new_lines.end());
        cursor_row += paste_lines.size() - 1;
        // Cursor at the end of the inserted block
        cursor_col = utf8::distance(paste_lines.back().begin(), paste_lines.back().end());
        invalidate_max_width_cache();
    }

    update_scroll_areas();
    scroll_to_cursor();
    redraw();
    if (change_callback) {
        change_callback(text_multiline());
    }
}

// Methods for working with scrolling
void input::update_scroll_areas()
{
    if (input_view_ != input_view::multiline) {
        return;
    }

    auto control_pos = position();
    auto border_width = theme_dimension(tcn, tv_border_width, theme_);
    auto font_ = theme_font(tcn, tv_font, theme_);
    int line_height = font_.size;
    
    // Creating a graphical context for measuring text
    
    // 1. We calculate the maximum line width (using cache)
    int max_width = get_max_line_width();

    // 2. We calculate the size of the text area
    int content_width = control_pos.width() - border_width * 2 - SCROLL_SIZE;
    int content_height = control_pos.height() - border_width * 2 - SCROLL_SIZE;

    // 3. First we count the vertical scrollbar
    int total_height = static_cast<int>(lines_.size()) * line_height;
    int vert_area = std::max(0, total_height - content_height - 4);
    vert_scroll->set_area(vert_area);
    bool need_vert_scroll = total_height > content_height;
    
    // 4. Now we are counting the horizontal scrollbar
    int hor_area = std::max(0, max_width - content_width - 4);
    hor_scroll->set_area(hor_area);
    bool need_hor_scroll = max_width > content_width;
    
    // 5. If you need a vertical scroll after the horizontal scroll appears, we recalculate it.
    if (need_hor_scroll) {
        vert_area = std::max(0, total_height - content_height);
        vert_scroll->set_area(vert_area);
    }

    update_scroll_visibility();
}

void input::on_vert_scroll(scroll_state ss, int32_t v) {
    scroll_offset_y = v;
    redraw();
}

void input::on_hor_scroll(scroll_state ss, int32_t v) {
    scroll_offset_x = v;
    redraw();
}

void input::update_scroll_visibility() {
    if (input_view_ != input_view::multiline) {
        return;
    }

    auto control_pos = position();
    auto border_width = theme_dimension(tcn, tv_border_width, theme_);
    auto font_ = theme_font(tcn, tv_font, theme_);

    int line_height = font_.size;
    int total_height = static_cast<int>(lines_.size()) * line_height;
    int content_height = control_pos.height() - border_width * 2 - SCROLL_SIZE;

    // Calculating the maximum row width (using cache)
    int max_width = get_max_line_width();
    int content_width = control_pos.width() - border_width * 2 - SCROLL_SIZE;

    // Showing/hiding the vertical scroll
    bool need_vert_scroll = total_height > content_height;
    if (need_vert_scroll != vert_scroll->showed()) {
        if (need_vert_scroll) {
            vert_scroll->show();
        } else {
            vert_scroll->hide();
        }
    }

    // Showing/hiding the horizontal scroll
    bool need_hor_scroll = max_width > content_width;
    if (need_hor_scroll != hor_scroll->showed()) {
        if (need_hor_scroll) {
            hor_scroll->show();
        } else {
            hor_scroll->hide();
        }
    }
}

// Cache management for performance
int input::get_max_line_width()
{
    if (!max_width_dirty_ && cached_max_width_ >= 0) {
        return cached_max_width_;
    }
    
    auto parent__ = parent_.lock(); if (!parent__) return 0;
    system_context ctx = parent__->context(); graphic mem_gr(ctx);
    mem_gr.init({ 0, 0, position_.width(), position_.height() }, 0 );

    auto font_ = theme_font(tcn, tv_font, theme_);
    int max_width = 0;
    
    for (const auto& line : lines_) {
        auto text_width = get_text_width(mem_gr, line, line.size(), font_);
        max_width = std::max(max_width, text_width);
    }
    
    cached_max_width_ = max_width + INPUT_HORIZONTAL_INDENT * 2;
    max_width_dirty_ = false;
    
    return cached_max_width_;
}

void input::invalidate_max_width_cache()
{
    max_width_dirty_ = true;
    cached_max_width_ = -1;
}

// Auto-scroll functions for mouse selection
void input::start_auto_scroll(bool up)
{
    if (auto_scroll_timer_ && !auto_scroll_active_) {
        auto_scroll_active_ = true;
        auto_scroll_up_ = up;
        auto_scroll_timer_->start(40); // 40ms interval (25 lines per sec)
    }
}

void input::stop_auto_scroll()
{
    if (auto_scroll_timer_ && auto_scroll_active_) {
        auto_scroll_active_ = false;
        auto_scroll_timer_->stop();
    }
}

void input::on_auto_scroll()
{
    if (!auto_scroll_active_ || !selecting || input_view_ != input_view::multiline) {
        auto_scroll_active_ = false;
        return;
    }
    
    if (auto_scroll_up_) {
        // Прокручиваем вверх на одну строку
        if (cursor_row > 0) {
            --cursor_row;
            cursor_col = std::min(cursor_col, static_cast<size_t>(utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end())));
            select_end_row = cursor_row;
            select_end_col = cursor_col;
            scroll_to_cursor();
            redraw();
        } else {
            // Если уже на первой строке, двигаем курсор в начало строки
            cursor_col = 0;
            select_end_row = cursor_row;
            select_end_col = cursor_col;
            scroll_to_cursor();
            redraw();
            auto_scroll_active_ = false;
        }
    } else {
        // Прокручиваем вниз на одну строку
        if (cursor_row + 1 < lines_.size()) {
            ++cursor_row;
            cursor_col = std::min(cursor_col, static_cast<size_t>(utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end())));
            select_end_row = cursor_row;
            select_end_col = cursor_col;
            scroll_to_cursor();
            redraw();
        } else {
            // Если уже на последней строке, двигаем курсор в конец строки
            cursor_col = utf8::distance(lines_[cursor_row].begin(), lines_[cursor_row].end());
            select_end_row = cursor_row;
            select_end_col = cursor_col;
            scroll_to_cursor();
            redraw();
            auto_scroll_active_ = false;
        }
    }
}

void input::scroll_to_cursor()
{
    if (input_view_ != input_view::multiline)
    {
        return;
    }
    if (lines_.empty() || cursor_row >= lines_.size())
    {
        return;
    }

    const auto& line = lines_[cursor_row];
    
    auto control_pos = position();
    auto border_width = theme_dimension(tcn, tv_border_width, theme_);
    auto font_ = theme_font(tcn, tv_font, theme_);
    int line_height = font_.size;
    
    int content_height = control_pos.height() - border_width * 2 - SCROLL_SIZE;
    int content_width = control_pos.width() - border_width * 2 - SCROLL_SIZE;
    
    // We take into account the place for scrollbars
    bool show_vert_scroll = vert_scroll->showed();
    bool show_hor_scroll = hor_scroll->showed();

    int visible_left = scroll_offset_x;
    int visible_right = visible_left + content_width - 1;
    const int cursor_extra = 8;

    // Calculating the cursor position in pixels
    int cursor_y = static_cast<int>(cursor_row) * line_height;
    int cursor_x = 0;
    // Calculating the horizontal cursor position
    size_t max_col = line.empty() ? 0 : utf8::distance(line.begin(), line.end());
    size_t safe_cursor_col = std::min(cursor_col, max_col);
    size_t cursor_byte = line.empty() ? 0 : get_byte_pos_for_char_pos(line, safe_cursor_col);
    
    auto parent__ = parent_.lock();
    if (!parent__) return;
    system_context ctx = parent__->context();
    graphic mem_gr(ctx);
    mem_gr.init({ 0, 0, position_.width() - border_width * 2, position_.height() - border_width * 2 }, theme_color(tcn, tv_background, theme_));

    cursor_x = mem_gr.measure_text(line.substr(0, cursor_byte), font_).right;
    int line_width = mem_gr.measure_text(line, font_).right;
    
    if (safe_cursor_col == max_col) {
        int new_scroll = std::max(0, line_width + cursor_extra - content_width);
        hor_scroll->set_scroll_pos(new_scroll);
    } else {
        if (cursor_x < visible_left) {
            hor_scroll->set_scroll_pos(cursor_x);
        } else if (cursor_x + cursor_extra > visible_right) {
            hor_scroll->set_scroll_pos(cursor_x - content_width + cursor_extra);
        }
    }
    
    // Checking if you need to scroll vertically.
    int visible_top = scroll_offset_y;
    int visible_bottom = visible_top + content_height;
    int total_height = static_cast<int>(lines_.size()) * line_height;
    int max_scroll = std::max(0, total_height - content_height);
    
    if (cursor_row == lines_.size() - 1) {
        vert_scroll->set_scroll_pos(max_scroll);
    } else if (cursor_y < visible_top) {
        vert_scroll->set_scroll_pos(cursor_y);
    } else if (cursor_y + line_height > visible_bottom) {
        int new_scroll = cursor_y + line_height - content_height;
        if (new_scroll < 0) new_scroll = 0;
        if (new_scroll > max_scroll) new_scroll = max_scroll;
        vert_scroll->set_scroll_pos(new_scroll);
    }
}

}
