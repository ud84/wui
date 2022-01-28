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

#include <boost/nowide/convert.hpp>
#include <utf8/utf8.h>

namespace wui
{

static const int32_t input_horizontal_indent = 5;

input::input(const std::string &text__, input_view input_view__, std::shared_ptr<i_theme> theme__)
    : input_view_(input_view__),
    text_(text__),
    change_callback(),
    theme_(theme__),
    position_(),
    cursor_position(0), select_start_position(0), select_end_position(0),
    parent(),
    my_subscriber_id(-1),
    timer_(std::bind(&input::redraw_cursor, this)),
    showed_(true), enabled_(true),
    focused_(false),
    focusing_(true),
    cursor_visible(false),
    selecting(false),
    left_shift(0)
{
}

input::~input()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(shared_from_this());
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

    /// Draw the frame
    gr.draw_rect(position(),
        !focused_ ? theme_color(tc, tv_border, theme_) : theme_color(tc, tv_focused_border, theme_),
        theme_color(tc, tv_background, theme_),
        theme_dimension(tc, tv_border_width, theme_),
        theme_dimension(tc, tv_round, theme_));

    auto font_ = theme_font(tc, tv_font, theme_);

    /// Create memory dc for text and selection bar
    auto full_text_width = get_text_width(gr, text_, text_.size(), font_) + 1; 
    auto text_height = font_.size;

#ifdef _WIN32
    system_context ctx = { 0, gr.drawable() };
#elif __linux__
    system_context ctx = { 0 };
    auto parent_ = parent.lock();
    if (parent_)
    {
        ctx = { parent_->context().display, parent_->context().connection, parent_->context().screen, gr.drawable() };
    }
#endif
    graphic mem_gr(ctx);
    mem_gr.init(rect{ 0, 0, full_text_width, text_height }, theme_color(tc, tv_background, theme_));

    /// Draw the selection bar
    if (select_start_position != select_end_position)
    {
        auto start_coordinate = get_text_width(mem_gr, text_, select_start_position, font_);
        auto end_coordinate = get_text_width(mem_gr, text_, select_end_position, font_);

        mem_gr.draw_rect(rect{ start_coordinate, 0, end_coordinate, text_height }, theme_color(tc, tv_selection, theme_));
    }

    /// Draw the text
    mem_gr.draw_text(rect{ 0 }, text_, theme_color(tc, tv_text, theme_), font_);

    /// Draw the cursor
    auto cursor_coordinate = get_text_width(mem_gr, text_, cursor_position, font_);
    mem_gr.draw_line(rect{ cursor_coordinate, 0, cursor_coordinate, text_height },
        cursor_visible ? theme_color(tc, tv_cursor, theme_) :
        (select_start_position != select_end_position && cursor_position >= select_start_position && cursor_position <= select_end_position ? theme_color(tc, tv_selection, theme_) : theme_color(tc, tv_background, theme_)));
    
    while (cursor_coordinate - left_shift >= position_.width() - input_horizontal_indent * 2)
    {
        left_shift += 10;
    }

    while (left_shift > cursor_coordinate)
    {
        left_shift -= 10;
    }

    int32_t input_vertical_indent = position_.height() > text_height ? (position_.height() - text_height) / 2 : 0;
    
    gr.draw_graphic(rect{ position().left + input_horizontal_indent,
            position().top + input_vertical_indent,
            position().width() - input_horizontal_indent * 2,
            position().height() - input_vertical_indent * 2 },
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
    auto parent_ = parent.lock();
    if (parent_)
    {
#ifdef _WIN32
        ctx = { parent_->context().hwnd, GetDC(parent_->context().hwnd) };
#elif __linux__
        ctx = parent_->context();
#endif
    }
    graphic mem_gr(ctx);
    mem_gr.init(position_, 0);

    auto font_ = theme_font(tc, tv_font, theme_);

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

#ifdef _WIN32
    ReleaseDC(ctx.hwnd, ctx.dc);
#endif

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

void input::receive_event(const event &ev)
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
                auto parent_ = parent.lock();
                if (parent_)
                {
                    set_cursor(parent_->context(), cursor::ibeam);
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
                selecting = false;

                auto parent_ = parent.lock();
                if (parent_)
                {
                    set_cursor(parent_->context(), cursor::default_);
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
                    case vk_left:
                        if (cursor_position > 0)
                        {
                            auto prev_position = cursor_position;

                            move_cursor_left();

                            update_select_positions(ev.keyboard_event_.modifier == vk_shift, prev_position, cursor_position);
                            redraw();
                        }
                    break;
                    case vk_right:
                        if (cursor_position < text_.size())
                        {
                            auto prev_position = cursor_position;

                            move_cursor_right();

                            update_select_positions(ev.keyboard_event_.modifier == vk_shift, prev_position, cursor_position);
                            redraw();
                        }
                    break;
                    case vk_home:
                        update_select_positions(ev.keyboard_event_.modifier == vk_shift, cursor_position, 0);
                        cursor_position = 0;
                        redraw();
                    break;
                    case vk_end:
                        if (!text_.empty())
                        {
                            update_select_positions(ev.keyboard_event_.modifier == vk_shift, cursor_position, text_.size());

                            cursor_position = text_.size();

                            redraw();
                        }
                    break;
                    case vk_back:
                        if (cursor_position > 0)
                        {
                            if (!clear_selected_text())
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
                        if (!text_.empty())
                        {
                            if (!clear_selected_text())
                            {
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
                }
            break;
            case keyboard_event_type::up:
                timer_.start(500);
                if (ev.keyboard_event_.key[0] == vk_shift)
                {
                    selecting = false;
                }
            break;
            case keyboard_event_type::key:
                if (input_view_ == input_view::singleline && ev.keyboard_event_.key[0] == vk_return)
                {
                    return;
                }

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
                    return buffer_paste();
                }
                else if (ev.keyboard_event_.key[0] == 0x1)  // ctrl + a
                {
                    return select_all();
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
}

void input::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent);
}

rect input::position() const
{
    return get_control_position(position_, parent);
}

void input::set_parent(std::shared_ptr<window> window_)
{
    parent = window_;
    my_subscriber_id = window_->subscribe(std::bind(&input::receive_event, this, std::placeholders::_1),
        static_cast<event_type>(static_cast<uint32_t>(event_type::mouse) | static_cast<uint32_t>(event_type::keyboard)),
        shared_from_this());
}

void input::clear_parent()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->unsubscribe(my_subscriber_id);
    }
    parent.reset();
}

bool input::topmost() const
{
    return false;
}

void input::set_focus()
{
    if (focusing_ && enabled_ && showed_)
    {
        focused_ = true;

        redraw();

        timer_.start(500);
    }
}

bool input::remove_focus()
{
    focused_ = false;

    cursor_visible = false;

    selecting = false;
    select_start_position = 0; select_end_position = 0;

    timer_.stop();

    redraw();

    return true;
}

bool input::focused() const
{
    return focused_;
}

bool input::focusing() const
{
    return enabled_ && showed_ && focusing_;
}

void input::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
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
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position(), true);
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

void input::set_text(const std::string &text__)
{
    text_ = text__;
    redraw();
}

std::string input::text() const
{
    return text_;
}

void input::set_input_view(input_view input_view__)
{
    input_view_ = input_view__;
}

void input::set_change_callback(std::function<void(const std::string&)> change_callback_)
{
    change_callback = change_callback_;
}

void input::redraw()
{
    if (showed_)
    {
        auto parent_ = parent.lock();
        if (parent_)
        {
            parent_->redraw(position());
        }
    }
}

void input::redraw_cursor()
{
    cursor_visible = !cursor_visible;
    redraw();
}

#ifdef _WIN32
void input::buffer_copy()
{
    if (select_start_position == select_end_position)
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

    std::string copy_text = text_.substr(start, end - start);
    auto wide_str = boost::nowide::widen(copy_text);

    if (OpenClipboard(NULL))
    {
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, copy_text.size() + 2);
        if (hGlobal != NULL)
        {
            LPVOID lpText = GlobalLock(hGlobal);
            memcpy(lpText, wide_str.c_str(), copy_text.size() + 2);

            EmptyClipboard();
            GlobalUnlock(hGlobal);

            SetClipboardData(CF_UNICODETEXT, hGlobal);
        }
        CloseClipboard();
    }
}

void input::buffer_cut()
{
    if (select_start_position == select_end_position)
    {
        return;
    }

    buffer_copy();

    clear_selected_text();

    redraw();
}

void input::buffer_paste()
{
    if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
    {
        return;
    }

    if (!OpenClipboard(NULL))
    {
        return;
    }

    std::string paste_string;

    HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
    if (hglb)
    {
        wchar_t *lptstr = (wchar_t *)GlobalLock(hglb);
        if (lptstr)
        {
            paste_string = boost::nowide::narrow(lptstr);
            GlobalUnlock(hglb);
        }
    }
    CloseClipboard();

    clear_selected_text();

    text_.insert(cursor_position, paste_string);

    cursor_position += paste_string.size();

    redraw();

    if (change_callback)
    {
        change_callback(text_);
    }
}

#elif __linux__

void input::buffer_copy()
{

}

void input::buffer_cut()
{

}

void input::buffer_paste()
{

}

#endif

}
