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

namespace wui
{

static const int32_t input_horizontal_indent = 5;

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
    showed_(true), enabled_(true), topmost_(false),
    focused_(false),
    cursor_visible(false),
    selecting(false),
    left_shift(0)
{
    menu_->set_items({
            { 0, menu_item_state::normal, locale(tc, cl_cut).data(), "Ctrl+X", nullptr, {}, [this](int32_t i) { buffer_cut(); } },
            { 1, menu_item_state::normal, locale(tc, cl_copy).data(), "Ctrl+C", nullptr, {}, [this](int32_t i) { buffer_copy(); } },
            { 2, menu_item_state::normal, locale(tc, cl_paste).data(), "Ctrl+V", nullptr, {}, [this](int32_t i) { buffer_paste(); } }
        });
}

input::~input()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }
}

int32_t get_text_width(graphic &gr, std::string text, size_t text_length, const font &font_)
{
    text.resize(text_length);
    auto text_rect = gr.measure_text(text, font_);

    return text_rect.right;
}

void input::draw(graphic &gr, const rect &)
{
    if (!showed_ || position_.width() == 0 || position_.height() == 0 || position_.width() <= input_horizontal_indent * 2)
    {
        return;
    }

    auto control_pos = position();

    /// Draw the frame
    gr.draw_rect(control_pos,
        !focused_ ? theme_color(tcn, tv_border, theme_) : theme_color(tcn, tv_focused_border, theme_),
        theme_color(tcn, tv_background, theme_),
        theme_dimension(tcn, tv_border_width, theme_),
        theme_dimension(tcn, tv_round, theme_));

    auto font_ = theme_font(tcn, tv_font, theme_);
    if (input_view_ == input_view::password)
    {
#ifdef _WIN32
        font_.name = "Courier New";
#elif __linux__
        font_.name = "monospace";
#endif
    }

    /// Create memory dc for text and selection bar
    auto full_text_width = get_text_width(gr, text_, text_.size(), font_) + 2;
    auto text_height = font_.size;

    system_context ctx = { 0 };
    auto parent__ = parent_.lock();
    if (parent__)
    {
#ifdef _WIN32
        ctx = parent__->context();
#elif __linux__
        ctx = { parent__->context().display, parent__->context().connection, parent__->context().screen, gr.drawable() };
#endif
}
    graphic mem_gr(ctx);
    mem_gr.init({ 0, 0, full_text_width, text_height }, theme_color(tcn, tv_background, theme_));

    /// Draw the selection bar
    if (select_start_position != select_end_position)
    {
        auto start_coordinate = get_text_width(mem_gr, text_, select_start_position, font_);
        auto end_coordinate = get_text_width(mem_gr, text_, select_end_position, font_);

        mem_gr.draw_rect({ start_coordinate, 0, end_coordinate, text_height }, theme_color(tcn, tv_selection, theme_));
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
                text__.append("●");
            }
        }
        mem_gr.draw_text({ 0 }, text__, theme_color(tcn, tv_text, theme_), font_);
    }
            
    /// Draw the cursor
    if (cursor_visible)
    {
        auto cursor_coordinate = get_text_width(mem_gr, text_, cursor_position, font_);
        mem_gr.draw_line({ cursor_coordinate, 0, cursor_coordinate, text_height }, theme_color(tcn, tv_cursor, theme_));

        while (cursor_coordinate - left_shift >= position_.width() - input_horizontal_indent * 2)
        {
            left_shift += 10;
        }

        while (left_shift > cursor_coordinate)
        {
            left_shift -= 10;
        }
    }

    int32_t input_vertical_indent = position_.height() > text_height ? (position_.height() - text_height) / 2 : 0;
    
    gr.draw_graphic({ control_pos.left + input_horizontal_indent,
            control_pos.top + input_vertical_indent,
            control_pos.width() - input_horizontal_indent * 2,
            control_pos.height() - input_vertical_indent * 2 },
        mem_gr, left_shift, 0);
}

size_t input::calculate_mouse_cursor_position(int32_t x)
{
    if (text_.empty())
    {
        return 0;
    }

    x -= position().left + input_horizontal_indent - left_shift;

    system_context ctx = { 0 };
    auto parent__ = parent_.lock();
    if (parent__)
    {
        ctx = parent__->context();
    }
    graphic mem_gr(ctx);
    mem_gr.init(position_, 0);

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
}

void input::select_all()
{
    select_start_position = 0;
    select_end_position = text_.size();;

    redraw();
}

bool input::check_count_valid(size_t count)
{
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
    return s.find_first_not_of("-,.0123456789");
}

void input::receive_control_events(const event &ev)
{
    if (!showed_ || !enabled_)
    {
        return;
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
                }

                auto parent__ = parent_.lock();
                if (parent__)
                {
                    set_cursor(parent__->context(), cursor::default_);
                }
            }
            break;
            case mouse_event_type::left_down:
                cursor_position = calculate_mouse_cursor_position(ev.mouse_event_.x);

                selecting = true;
                select_start_position = cursor_position;
                select_end_position = select_start_position;

                redraw();
            break;
            case mouse_event_type::left_up:
                selecting = false;
                menu_->hide();
            break;
            case mouse_event_type::right_up:
                menu_->update_item({ 0, select_start_position != select_end_position && input_view_ != input_view::readonly && input_view_ != input_view::password ? menu_item_state::normal : menu_item_state::disabled,
                    locale(tc, cl_cut).data(), "Ctrl+X", nullptr, {}, [this](int32_t i) { buffer_cut(); parent_.lock()->set_focused(shared_from_this()); } });
                menu_->update_item({ 1, select_start_position != select_end_position && input_view_ != input_view::password ? menu_item_state::normal : menu_item_state::disabled,
                    locale(tc, cl_copy).data(), "Ctrl+C", nullptr, {}, [this](int32_t i) { buffer_copy(); parent_.lock()->set_focused(shared_from_this()); } });
                menu_->update_item({ 2, input_view_ != input_view::readonly ? menu_item_state::normal : menu_item_state::disabled,
                    locale(tc, cl_paste).data(), "Ctrl+V", nullptr, {}, [this](int32_t i) { buffer_paste(); parent_.lock()->set_focused(shared_from_this()); } });

                menu_->show_on_control(shared_from_this(), 0, ev.mouse_event_.x);
            break;
            case mouse_event_type::move:
                if (selecting)
                {    
                    auto measured_cursor_position = calculate_mouse_cursor_position(ev.mouse_event_.x);
                    if (cursor_position != measured_cursor_position)
                    {
                        cursor_position = measured_cursor_position;
                        select_end_position = cursor_position;

                        redraw();
                    }
                }
            break;
            case mouse_event_type::left_double:
                select_current_word(ev.mouse_event_.x);
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
                    return buffer_copy();
                }
                else if (ev.keyboard_event_.key[0] == 0x18) // ctrl + x
                {
                    return buffer_cut();
                }
                else if (ev.keyboard_event_.key[0] == 0x16) // ctrl + v
                {
                    if (input_view_ != input_view::readonly)
                    {
                        return buffer_paste();
                    }
                }
                else if (ev.keyboard_event_.key[0] == 0x1)  // ctrl + a
                {
                    return select_all();
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
                
                clear_selected_text();

                text_.insert(cursor_position, ev.keyboard_event_.key);
                
                cursor_position += ev.keyboard_event_.key_size;

                redraw();

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

void input::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent_);
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
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

    menu_->update_theme(theme_);
    redraw();
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
    text_ = text__;
    cursor_position = 0;

    redraw();

    if (change_callback)
    {
        change_callback(text_);
    }
}

std::string input::text() const
{
    return text_;
}

void input::set_input_view(input_view input_view__)
{
    input_view_ = input_view__;
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

    if (change_callback)
    {
        change_callback(text_);
    }
}

}
