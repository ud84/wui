//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/control/image.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

#ifdef _WIN32

void load_image_from_resource(WORD image_id, const std::wstring &resource_section, Gdiplus::Image **img)
{
    HINSTANCE h_inst = GetModuleHandle(NULL);
    HRSRC h_resource = FindResource(h_inst, MAKEINTRESOURCE(image_id), resource_section.c_str());
    if (!h_resource)
    {
        return;
    }

    DWORD image_size = ::SizeofResource(h_inst, h_resource);
    if (!image_size)
    {
        return;
    }

    const void* resource_data = ::LockResource(::LoadResource(h_inst, h_resource));
    if (!resource_data)
    {
        return;
    }

    HGLOBAL h_buffer = ::GlobalAlloc(GMEM_MOVEABLE, image_size);
    if (h_buffer)
    {
        void* p_buffer = ::GlobalLock(h_buffer);
        if (p_buffer)
        {
            CopyMemory(p_buffer, resource_data, image_size);

            IStream* p_stream = NULL;
            if (::CreateStreamOnHGlobal(h_buffer, FALSE, &p_stream) == S_OK)
            {
                *img = Gdiplus::Image::FromStream(p_stream);
                p_stream->Release();
            }

            ::GlobalUnlock(p_buffer);
        }
        ::GlobalFree(h_buffer);
    }
}

void load_image_from_file(const std::wstring &file_name, const std::wstring &images_path, Gdiplus::Image **img)
{
    *img = Gdiplus::Image::FromFile(std::wstring(images_path + L"\\" + file_name).c_str());
}

void free_image(Gdiplus::Image **img)
{
    if (*img)
    {
        delete *img;
        *img = nullptr;
    }
}

#endif

namespace wui
{

#ifdef _WIN32
image::image(int32_t resource_index_, std::shared_ptr<i_theme> theme__)
    : theme_(theme__),
    position_(),
    parent(),
    showed_(true),
    file_name(),
    resource_index(resource_index_),
    img(nullptr)
{
    load_image_from_resource(resource_index, theme_string(theme_value::images_path, theme_), &img);
}
#endif

image::image(const std::wstring &file_name_, std::shared_ptr<i_theme> theme__)
    : theme_(theme__),
    position_(),
    parent(),
    showed_(true),
    file_name(file_name_)
#ifdef _WIN32
    , resource_index(0),
    img(nullptr)
#endif
{
#ifdef _WIN32
    load_image_from_file(file_name_, theme_string(theme_value::images_path, theme_), &img);
#elif __linux__

#endif
}

image::~image()
{
#ifdef _WIN32
    free_image(&img);
#endif

    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(shared_from_this());
    }
}

void image::draw(graphic &gr_)
{
    if (!showed_)
    {
        return;
    }

#ifdef _WIN32
    if (img)
    {
        Gdiplus::Graphics gr(gr_.drawable());

        gr.DrawImage(
            img->Clone(),
            Gdiplus::Rect(position_.left, position_.top, position_.width(), position_.height()),
            0, 0, img->GetWidth(), img->GetHeight(),
            Gdiplus::UnitPixel,
            nullptr);
    }
#endif
}

void image::receive_event(const event &)
{
}

void image::set_position(const rect &position__)
{
    auto prev_position = position_;
    position_ = position__;

    if (showed_)
    {
        auto parent_ = parent.lock();
        if (parent_)
        {
            parent_->redraw(prev_position, true);
        }
    }
	
    redraw();
}

rect image::position() const
{
	return position_;
}

void image::set_parent(std::shared_ptr<window> window)
{
    parent = window;
}

void image::clear_parent()
{
    parent.reset();
}

void image::set_focus()
{
}

bool image::remove_focus()
{
    return true;
}

bool image::focused() const
{
    return false;
}

bool image::focusing() const
{
    return false;
}

void image::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

#ifdef _WIN32
    if (resource_index)
    {
        change_image(resource_index);
    }
    else
#endif
    if (!file_name.empty())
    {
        change_image(file_name);
    }
}

void image::show()
{
    showed_ = true;
    redraw();
}

void image::hide()
{
    showed_ = false;
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position_, true);
    }
}

bool image::showed() const
{
    return showed_;
}

void image::enable()
{
}

void image::disable()
{
}

bool image::enabled() const
{
    return true;
}

#ifdef _WIN32
void image::change_image(int32_t resource_index_)
{
    resource_index = resource_index_;

    free_image(&img);
    load_image_from_resource(resource_index, theme_string(theme_value::images_path, theme_), &img);
    redraw();
}
#endif

void image::change_image(const std::wstring &file_name_)
{
    file_name = file_name_;

#ifdef _WIN32
    free_image(&img);
    load_image_from_file(file_name, theme_string(theme_value::images_path, theme_), &img);
#endif

    redraw();
}

int32_t image::width() const
{
#ifdef _WIN32
    if (img)
    {
        return img->GetWidth();
    }
#endif
    return 0;
}

int32_t image::height() const
{
#ifdef _WIN32
    if (img)
    {
        return img->GetHeight();
    }
#endif
    return 0;
}

void image::redraw()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position_);
    }
}

}
