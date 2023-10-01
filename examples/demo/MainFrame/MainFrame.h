#pragma once

#include <wui/window/window.hpp>
#include <wui/control/button.hpp>

enum class Sheet
{
    Main,
    Window,
    Button,
    Input,
    List,
    Menu,
    Panel
};

class MainFrame
{
public:
    MainFrame();

    void Run();

    bool Runned() const;

private:
    static const int32_t WND_WIDTH = 800, WND_HEIGHT = 600;

    std::shared_ptr<wui::window> window;

    std::shared_ptr<wui::button> mainSheet, windowSheet, buttonSheet, inputSheet, listSheet, menuSheet, panelSheet;

    bool runned;

    Sheet sheet;

    void ReceiveEvents(const wui::event &ev);

    void UpdateSheets();
    void ShowMain();
    void ShowWindow();
    void ShowButton();
    void ShowInput();
    void ShowList();
    void ShowMenu();
    void ShowPanel();
};
