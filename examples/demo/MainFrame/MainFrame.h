// Copyright (c) 2021-2026 Intent Garden Org
// Distributed under the Boost Software License, Version 1.0.
#pragma once

#include <wui/window/window.hpp>
#include <wui/control/button.hpp>
#include <wui/control/text.hpp>
#include <wui/control/message.hpp>
#include <vector>

// Each page owns its controls for the lifetime of the showcase. Switching pages
// preserves edits and selection; geometry is recomputed without rebuilding widgets.
class MainFrame
{
public:
    MainFrame();
    ~MainFrame();
    void Run();
private:
    struct Item { std::shared_ptr<wui::i_control> control; wui::rect bounds; int page; };
    std::shared_ptr<wui::window> window = std::make_shared<wui::window>();
    std::vector<Item> items;
    std::vector<std::shared_ptr<wui::button>> tabs;
    std::shared_ptr<wui::text> status;
    std::shared_ptr<wui::window> child;
    std::unique_ptr<wui::message> message;
    int page = 0;
    int buildingPage = -1;
    std::function<void()> layoutDetail;
    template<class T> std::shared_ptr<T> Add(std::shared_ptr<T> control, wui::rect bounds)
    {
        items.push_back({control, bounds, buildingPage});
        if (buildingPage < 0 || buildingPage == page) window->add_control(control, bounds);
        return control;
    }
    std::shared_ptr<wui::text> Label(const std::string&, wui::rect, const char* theme = "text");
    std::shared_ptr<wui::button> Button(const std::string&, wui::rect, std::function<void()>, wui::button_view = wui::button_view::text);
    void Heading(const char*, const char*);
    void Card(wui::rect);
    void ShowPage(int);
    void Layout();
    void Overview();
    void Windows();
    void Buttons();
    void Inputs();
    void Lists();
    void Menus();
    void Layouts();
    void Notify(const std::string&);
    void Dialog(wui::message_icon, wui::message_button);
};
