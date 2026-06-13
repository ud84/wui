//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#pragma once

#include <string>

namespace wui
{

enum class error_type
{
    ok = 0,

    file_not_found,
    invalid_json,
    invalid_value,
    system_error,
    no_handle,
    already_started
};


class error
{
    error_type type{ error_type::ok };
    std::string component;
    std::string message;
public:
    error(error_type type_, std::string_view component_,
        std::string_view message_ = "") : type(type_), component(component_),
        message(message_)
    {
    }

    error() = default;

    inline bool operator==(const error &lv)
    {
        return lv.type == type && lv.component == component;
    }

    [[nodiscard]] inline bool is_ok() const
    {
        return error_type::ok == type;
    }

    inline void reset()
    {
        type = error_type::ok;
        component.clear();
        message.clear();
    }

    [[nodiscard]] inline error_type get_type() const
    {
        return type;
    }

    [[nodiscard]] inline std::string get_component() const
    {
        return component;
    }

    [[nodiscard]] inline std::string get_message() const
    {
        return message;
    }

    inline void set(error_type type_, std::string_view component_,
        std::string_view message_ = "")
    {
        type = type_;
        component = component_;
        message = message_;
    }

    [[nodiscard]] std::string str() const
    {
        return "WUI error :: type: " + get_text(type) + ", component: " + component + ", message: " + message;
    }

    [[nodiscard]] static std::string get_text(error_type t);
};

}
