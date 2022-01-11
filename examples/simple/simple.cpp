// wui.cpp : Defines the entry point for the application.
//

#include <wui/theme/theme.hpp>
#include <wui/window/window.hpp>
#include <wui/control/button.hpp>
#include <wui/control/input.hpp>
#include <wui/control/image.hpp>

#ifdef _WIN32
#include <Resource.h>
#include <gdiplus.h>
#endif

struct PluggedWindow : public std::enable_shared_from_this<PluggedWindow>
{
    std::shared_ptr<wui::window> &parentWindow;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::button> plugButton, unplugButton;

    std::weak_ptr<wui::button> createPluggedButton;

    bool plugged;

    void Plug()
    {
        parentWindow->add_control(window, wui::rect{ 20, 30, 190, 190 });

        plugButton->disable();
        unplugButton->enable();

        plugged = !plugged;
    }

    void Unplug()
    {
        parentWindow->remove_control(window);
        Init();
		
        plugButton->enable();
        unplugButton->disable();

        plugged = !plugged;
    }

    void SetPluggedButton(std::shared_ptr<wui::button> &createPluggedButton_)
    {
        createPluggedButton = createPluggedButton_;
    }

    void Init()
    {
        window->init(L"Child window plugged!", wui::rect{ 20, 30, 190, 190 }, wui::window_style::pinned, [this]() {
            if (createPluggedButton.lock())
                createPluggedButton.lock()->enable();
        });
    }

    PluggedWindow(std::shared_ptr<wui::window> &parentWindow_)
        : parentWindow(parentWindow_),
        window(new wui::window()),
        plugButton(new wui::button(L"Plug Window", std::bind(&PluggedWindow::Plug, this))),
        unplugButton(new wui::button(L"Unplug Window", std::bind(&PluggedWindow::Unplug, this))),
        createPluggedButton(),
        plugged(true)
    {
        window->add_control(unplugButton, wui::rect{ 10, 40, 110, 65 });
        window->add_control(plugButton, wui::rect{ 10, 85, 110, 110 });

        window->set_pin_callback([this]() { if (plugged) Unplug(); else Plug(); });

        plugButton->disable();

        parentWindow->add_control(window, wui::rect{ 20, 30, 190, 190 });

        Init();
    }
};


#ifdef _WIN32
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#elif __linux__
int main(int argc, char *argv[])
{
    const std::wstring IDB_ACCOUNT = L"";
#endif
    bool runned = true;

    wui::set_default_theme(wui::theme::dark);

    std::shared_ptr<wui::window> window(new wui::window());

    std::shared_ptr<wui::image> accountImage(new wui::image(IDB_ACCOUNT));
    window->add_control(accountImage, wui::rect{ 250, 100, 314, 164 });

    std::shared_ptr<PluggedWindow> pluggedWindow(new PluggedWindow(window));
    std::shared_ptr<wui::button> createPluggedButton(new wui::button(L"Create plugged window", []() {}));
    createPluggedButton->set_callback([&window, &pluggedWindow, &createPluggedButton]() {
        pluggedWindow = std::shared_ptr<PluggedWindow>(new PluggedWindow(window));
        pluggedWindow->SetPluggedButton(createPluggedButton);
        createPluggedButton->disable(); });
    createPluggedButton->disable();
    pluggedWindow->SetPluggedButton(createPluggedButton);

    window->add_control(createPluggedButton, wui::rect{ 270, 50, 380, 75 });
    
    std::shared_ptr<wui::input> nameInput(new wui::input());
    window->add_control(nameInput, wui::rect{ 10, 250, 400, 275 });

    std::shared_ptr<wui::window> dialog(new wui::window());

    std::shared_ptr<wui::button> okButton(new wui::button(L"OK", [window, &dialog]()
    {
        window->block();

        std::shared_ptr<wui::button> dialogButton(new wui::button(L"Close", [&dialog]() { dialog->destroy(); }));
        dialog->add_control(dialogButton, wui::rect{ 10, 200, 100, 235 });

        dialog->init(L"Modal dialog", wui::rect{ 50, 50, 350, 350 }, wui::window_style::dialog, [window, &dialog]() { window->unlock(); /*dialog.reset();*/ });
    }));

    auto redButtonTheme = wui::make_custom_theme();
    redButtonTheme->set_color(wui::theme_value::button_calm, wui::make_color(205, 15, 20));
    redButtonTheme->set_color(wui::theme_value::button_active, wui::make_color(235, 15, 20));
    redButtonTheme->set_color(wui::theme_value::button_border, wui::make_color(200, 215, 200));
    redButtonTheme->set_color(wui::theme_value::button_focused_border, wui::make_color(20, 215, 20));
    redButtonTheme->set_color(wui::theme_value::button_text, wui::make_color(190, 205, 190));
    redButtonTheme->set_color(wui::theme_value::button_disabled, wui::make_color(180, 190, 180));
    redButtonTheme->set_dimension(wui::theme_value::button_round, 0);
    redButtonTheme->set_dimension(wui::theme_value::button_font_size, 14);
    redButtonTheme->set_string(wui::theme_value::images_path, L"IMAGES_DARK");
    redButtonTheme->set_string(wui::theme_value::button_font_name, L"Segoe UI");
    std::shared_ptr<wui::button> cancelButton(new wui::button(L"Cancel", [window]() { window->destroy(); }, wui::button_view::image_right_text, IDB_ACCOUNT, 24, redButtonTheme));

    std::shared_ptr<wui::button> darkThemeButton(new wui::button(L"Set the dark theme", [window, &pluggedWindow, dialog]() { wui::set_default_theme(wui::theme::dark); window->update_theme(); pluggedWindow->window->update_theme(); dialog->update_theme(); }));
    window->add_control(darkThemeButton, wui::rect{ 140, 350, 150, 375 });
	
    std::shared_ptr<wui::button> whiteThemeButton(new wui::button(L"Set the white theme", [window, &pluggedWindow, dialog]() { wui::set_default_theme(wui::theme::white); window->update_theme(); pluggedWindow->window->update_theme(); dialog->update_theme(); }));
    window->add_control(whiteThemeButton, wui::rect{ 290, 350, 380, 375 });

    window->add_control(okButton, wui::rect{ 240, 450, 350, 480 });
    window->add_control(cancelButton, wui::rect{ 370, 450, 480, 480 });

    /*auto fileImageTheme = wui::make_custom_theme();
    fileImageTheme->set_string(wui::theme_value::images_path, L"d:\\");
    std::shared_ptr<wui::image> fileImage(new wui::image(L"g620.png", fileImageTheme));
    window->add_control(fileImage, wui::rect{ 180, 200, 344, 344 });*/

    window->set_min_size(100, 100);

    window->set_size_change_callback([&nameInput, &okButton, &cancelButton](int32_t w, int32_t h) {
        nameInput->set_position({ 10, 250, w - 10, 275 });
        okButton->set_position({ w - 250, h - 50, w - 150, h - 20});
        cancelButton->set_position({ w - 120, h - 50, w - 20, h - 20 });
    });

    window->init(L"Welcome to WUI!", wui::rect{ 100, 100, 600, 600 }, wui::window_style::frame, [&runned]() {
#ifdef _WIN32
        PostQuitMessage(IDCANCEL);
#elif __linux__
        runned = false;
#endif
    });
	
#ifdef _WIN32
    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //Gdiplus::GdiplusShutdown(gdiplusToken);
    return (int) msg.wParam;
#elif __linux__
    // Wait for main window
    while (runned)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
#endif
}
