//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#include <wui/control/text.hpp>
#include <wui/window/window.hpp>
#include <wui/theme/theme.hpp>
#include <wui/system/tools.hpp>
#include <vector>

#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

namespace wui
{

a_text::a_text(std::string_view text__,
    hori_alignment hori_alignment__, vert_alignment vert_alignment__,
    std::string_view theme_control_name,
    std::shared_ptr<i_theme> theme__, bool clip__)
    : tcn(theme_control_name),
    theme_(theme__),
    text_(text__),
    hori_alignment_(hori_alignment__),
    vert_alignment_(vert_alignment__),
    clip_(clip__)
{
}

a_text::~a_text() {}

text::text(std::string_view text__,
    hori_alignment hori_alignment__, vert_alignment vert_alignment__,
    std::string_view theme_control_name,
    std::shared_ptr<i_theme> theme__, bool clip__)
    : a_text(text__, hori_alignment__, vert_alignment__,
        theme_control_name, theme__, clip__)
{}

text::~text()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }
}

text_ex::text_ex(std::string_view text__,
    hori_alignment hori_alignment__, vert_alignment vert_alignment__,
    std::string_view theme_control_name,
    std::shared_ptr<i_theme> theme__, bool clip__)
    : a_text(text__, hori_alignment__, vert_alignment__,
        theme_control_name, theme__, clip__)
{}

text_ex::~text_ex()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }
}

int32_t a_text::get_font_size() const
{
    return theme_font(tcn, tv_font, theme_).size;
}

void a_text::set_space_coeff(const double val) noexcept
{
    space_coeff_ = (val >= 0.6 ? val : 0.6);
}

double a_text::get_space_coeff() const noexcept
{
    return space_coeff_;
}

void a_text::make_lines(const std::string_view text, std::vector<std::string> &lines)
{
    lines.emplace_back();
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (c == '\r') {
            if (i + 1 < text.size() && '\n' == text[i + 1]) {
                ++i;
            }
            lines.emplace_back();
            continue;
        }
        if (c == '\n') {
            lines.emplace_back();
            continue;
        }
        if (c == '\t') {
            lines.back().push_back(' ');
            lines.back().push_back(' ');
            lines.back().push_back(' ');
            lines.back().push_back(' ');
            continue;
        }
        lines.back().push_back(c);
    }

    const char last = text.back();
    if ((last == '\n' || last == '\r') && !lines.empty()) {
        lines.pop_back();
    }
}

int32_t text::measure_text_line(const std::string &text__, const font& font__)
{
    return measure_text(text__, font__).width();
}

int32_t text_ex::measure_text_line(const std::string& text__, const font& font__)
{
#ifdef _WIN32
    return measure_text_gdiplus(text__, font__).width();
#else
    return measure_text(text__, font__).width();
#endif
}

rect a_text::get_preferred_size()
{
    const auto font_ = theme_font(tcn, tv_font, theme_);
    if (text_.empty())
    {
        return rect{ 0, 0, 0, font_.size };
    }

    std::vector<std::string> lines;
    make_lines(text_, lines);
    if (lines.empty())
    {
        return rect{ 0, 0, 0, font_.size };
    }

    int32_t width_max{ };
    for (auto& line : lines)
    {
        const int32_t width = measure_text_line(line, font_);
        if (width_max < width)
        {
            width_max = width;
        }
    }
    const auto line_height = font_.size;
    const auto line_space = static_cast<int32_t>(line_height * space_coeff_);
    const int32_t height_max = line_height + static_cast<int32_t>((lines.size() - 1)) * line_space;
    return rect{ 0, 0, width_max, height_max };
}

int32_t text::truncate_and_measure_text
(
    std::string& text__, const font& font__, const int32_t width__,
    graphic* gr
)
{
    truncate_line(text__, gr, font__, width__);
    return measure_text(text__, font__, gr).width();
}

int32_t text_ex::truncate_and_measure_text
(
    std::string& text__, const font& font__, const int32_t width__,
    graphic* gr
)
{
#ifdef _WIN32
    truncate_line_gdiplus(text__, gr, font__, width__);
    return measure_text_gdiplus(text__, font__, gr).width();
#else
    truncate_line(text__, gr, font__, width__);
    return measure_text(text__, font__, gr).width();
#endif
}

void a_text::update_text(graphic* gr, const bool clip__)
{
    update_ = false;
    lines_.clear();
    if (text_.empty() || position_.is_null())
    {
#ifdef _UI_CHECK
        text_position_ = text_.empty() ?
            rect{ 0, 0, 0, theme_font(tcn, tv_font, theme_).size } : position_;
#endif
        return;
    }

    std::vector<std::string> tls;
    make_lines(text_, tls);
    if (tls.empty())
    {
#ifdef _UI_CHECK
        text_position_ = rect{ 0, 0, 0, theme_font(tcn, tv_font, theme_).size };
#endif
        return;
    }

    const auto font_ = theme_font(tcn, tv_font, theme_);
    const auto line_height = font_.size;
    const auto control_pos = position();
    const auto line_space = static_cast<int32_t>(line_height * space_coeff_);

    int32_t line_top{ control_pos.top };
#ifdef _UI_CHECK
    int32_t tmp_top{ position_.top };
#endif
    switch (vert_alignment_)
    {
        case vert_alignment::top:
            // line_top = control_pos.top;
            break;
        case vert_alignment::center:
        {
            auto t = static_cast<int32_t>((control_pos.height()
                - line_height - (tls.size() - 1) * line_space) / 2);
            if (t < 0) t = 0;
            line_top += t;
#ifdef _UI_CHECK
            tmp_top += t;
#endif
        }
        break;
        case vert_alignment::bottom:
        {
            auto t = static_cast<int32_t>(line_height * (1 + (tls.size() - 1) * space_coeff_));
            if (t > control_pos.height()) t = control_pos.height();
            line_top = control_pos.bottom - t;
#ifdef _UI_CHECK
            tmp_top = position_.bottom - t;
#endif
        }
        break;
    }

#ifdef _UI_CHECK
    const auto line_top_shift = line_top - line_height;
    int32_t line_top_prev{ }, width_max{ }, tmp_left{ position_.right };
#endif
    for (auto& str : tls)
    {
        const int32_t width =
            truncate_and_measure_text(str, font_, control_pos.width(), gr);

        int32_t left = control_pos.left;
        switch (hori_alignment_)
        {
            case hori_alignment::left:
                // do nothing
#ifdef _UI_CHECK
                tmp_left = position_.left;
#endif
                break;
            case hori_alignment::center:
            {
                // NB: get_control_position() не меняет position_.width()
                const auto t = (control_pos.width() - width) / 2;
                left += t;
#ifdef _UI_CHECK
                tmp_left = std::min(tmp_left, position_.left + t);
#endif
            }
            break;
            case hori_alignment::right:
            {
                const auto t = (control_pos.width() - width);
                left += t;
#ifdef _UI_CHECK
                tmp_left = std::min(tmp_left, position_.left + t);
#endif
            }
            break;
        }

        graphic::text_line tl;
        tl.str = std::move(str);
        tl.rc = rect{ left, line_top, 0, 0 };
        lines_.emplace_back(tl);

#ifdef _UI_CHECK
        width_max = std::max(width_max, width);
        line_top_prev = line_top;
#endif
        line_top += line_space;
        if (line_top + line_height > control_pos.bottom)
        {
            if(clip__)
            {
                if (line_top < control_pos.bottom)
                {
                    continue;
                }
#ifdef _UI_CHECK
                line_top_prev = control_pos.height() + line_top_shift; // for full clip box
#endif
            }
            break;
        }
    }

#ifdef _UI_CHECK
    //NB: (line_top_prev - line_top_shift) := height max
    text_position_ = rect{ tmp_left, tmp_top, tmp_left + width_max, tmp_top + line_top_prev - line_top_shift };
#endif
}

void text::draw_text(graphic& gr)
{
    const auto font_ = theme_font(tcn, tv_font, theme_);
    const auto color_ = theme_color(tcn, tv_color, theme_);
    //if(clip_)
    //{
        // предпочтительнее для разных языков?
        gr.draw_text_clip_rgb(position(), lines_, color_, font_, clip_);
    //}
    //else
    //{
    //    for (auto& line : lines_)
    //    {
    //        gr.draw_text(line.rect, line.str, color_, font_);
    //    }
    //}
}

void text_ex::draw_text(graphic& gr)
{
    const auto font_ = theme_font(tcn, tv_font, theme_);
    gr.draw_text_clip(position(), lines_, theme_color(tcn, tv_color, theme_), font_, clip_);
}

void a_text::draw(graphic& gr, const rect&)
{
    if (update_)
    {
        update_text(&gr, clip_);
    }

    if (!showed_ || position_.is_null() || text_.empty())
    {
        return;
    }

#ifdef _UI_CHECK
    {
        constexpr int32_t border = 1;
        constexpr int32_t rnd = 0;

        gr.draw_rect(position(),
            theme_color(window::tc, window::tv_border, theme_),
            theme_color(window::tc, window::tv_background, theme_),
            border, rnd);

        const rect r2 = position_text();

        // test: FillRect > DrawRoundBox
        //gr.draw_rect(r2, make_color(255, 0, 0));

        gr.draw_rect(r2,
            make_color(0, 100, 255),
            theme_color(window::tc, window::tv_background, theme_),
            border, rnd);

        // test: FillRect < DrawRoundBox and alpha if clip (gdi+ draw_text() and cairo)
        //gr.draw_rect(r2, make_color(0, 105, 155));
    }
#endif

    draw_text(gr);
}

void a_text::set_position(const rect& position__)
{
    position_ = position__;

    if (!clip_)
    {
        const auto font_ = theme_font(tcn, tv_font, theme_);
        if (position__.height() < font_.size)
        {
            position_.bottom = position_.top + font_.size;
        }
    }

    update_ = true;
}

rect a_text::position() const
{
    return get_control_position(position_, parent_);
}

#ifdef _UI_CHECK
// draw DEBUG rect
rect a_text::position_text() const
{
    rect r = text_position_;
    if (clip_)
    {
        rect::min(r, r, position_);
    }
    return get_control_position(r, parent_);
}
#endif

void a_text::set_parent(std::shared_ptr<window> window)
{
    parent_ = window;
}

std::weak_ptr<window> a_text::parent() const
{
    return parent_;
}

void a_text::clear_parent()
{
    parent_.reset();
}

void a_text::set_topmost(bool yes)
{
    topmost_ = yes;
}

bool a_text::topmost() const
{
    return topmost_;
}

bool a_text::focused() const
{
    return false;
}

bool a_text::focusing() const
{
    return false;
}

error a_text::get_error() const
{
    return {};
}

void a_text::update_theme_control_name(std::string_view theme_control_name)
{
    tcn = theme_control_name;
    update_theme(theme_);
}

void a_text::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;
    update_ = true;
    redraw();
}

void a_text::show()
{
    showed_ = true;
    redraw();
}

void a_text::hide()
{
    showed_ = false;
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->redraw(position(), true);
    }
}

bool a_text::showed() const
{
    return showed_;
}

void a_text::enable()
{
}

void a_text::disable()
{
}

bool a_text::enabled() const
{
    return true;
}

std::string_view a_text::get_text() const
{
    return text_;
}

void a_text::set_alignment(hori_alignment hori_alignment__, vert_alignment vert_alignment__)
{
    hori_alignment_ = hori_alignment__;
    vert_alignment_ = vert_alignment__;
    update_ = true;
    redraw();
}

void a_text::redraw()
{
    if (showed_)
    {
        auto parent__ = parent_.lock();
        if (parent__)
        {
            parent__->redraw(position(), true);  // стираем актуальную область текста
        }
    }
}

}
