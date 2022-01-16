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

static const char * dark_json = "{ \"controls\": [ { \"type\": \"window\", \"background\": \"#131519\", \"text\": \"#f5f5f0\", \"active_button\": \"#3b3d41\", \"caption_font\": { \"name\": \"Segoe UI\", \"size\": 18 } }, { \"type\": \"image\", \"path\" : \"IMAGES_DARK\" }, { \"type\": \"button\", \"calm\": \"#06a5df\", \"active\": \"#1aafe9\", \"border\": \"#6b6b6b\", \"focused_border\": \"#dcd2dc\", \"text\": \"#f0f1f1\", \"disabled\": \"#a5a5a0\", \"round\": 0, \"font\": { \"name\": \"Segoe UI\", \"size\": 18 } }, { \"type\": \"input\", \"background\": \"#27292d\", \"text\": \"#f0ebf0\", \"selection\": \"#264f78\", \"cursor\": \"#d2d2d2\", \"border\": \"#8c8c8c\", \"focused_border\": \"#c8c8c8\", \"round\": 0, \"font\": { \"name\": \"Segoe UI\", \"size\": 18 } }, { \"type\": \"tooltip\", \"background\": \"#b4aabe\", \"border\": \"#f1f2f7\", \"text\": \"#061912\", \"round\": 0, \"text_indent\": 3, \"font\": { \"name\": \"Segoe UI\", \"size\": 16 } } ] } ";
static const char * white_json = "{ \"controls\": [ { \"type\": \"window\", \"background\": \"#f0f0f0\", \"text\": \"#191914\", \"active_button\": \"#dcdcdc\", \"font\": { \"name\": \"Segoe UI\", \"size\": 18 } }, { \"type\": \"image\", \"path\" : \"IMAGES_WHITE\" }, { \"type\": \"button\", \"calm\": \"#06a5df\", \"active\": \"#1aafe9\", \"border\": \"#b0b0b0\", \"focused_border\": \"#140a14\", \"text\": \"#181818\", \"disabled\": \"#cdcdc8\", \"round\": 0, \"font\": { \"name\": \"Segoe UI\", \"size\": 18 } }, { \"type\": \"input\", \"background\": \"#dcdcdc\", \"text\": \"#191914\", \"selection\": \"#99c9ef\", \"cursor\": \"#141414\", \"border\": \"#28788c\", \"focused_border\": \"#140a14\", \"round\": 0, \"font\": { \"name\": \"Segoe UI\", \"size\": 18 } }, { \"type\": \"tooltip\", \"background\": \"#f1f2f7\", \"border\": \"#767676\", \"text\": \"#061912\", \"round\": 0, \"text_indent\": 3, \"font\": { \"name\": \"Segoe UI\", \"size\": 16 } } ] } ";

struct PluggedWindow : public std::enable_shared_from_this<PluggedWindow>
{
    std::weak_ptr<wui::window> parentWindow;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::button> plugButton, unplugButton;

    std::weak_ptr<wui::button> createPluggedButton;

    bool plugged;

    void Plug()
    {
        if (parentWindow.lock())
            parentWindow.lock()->add_control(window, wui::rect{ 20, 30, 190, 190 });

        plugButton->disable();
        unplugButton->enable();

        plugged = !plugged;
    }

    void Unplug()
    {
        if (parentWindow.lock())
            parentWindow.lock()->remove_control(window);
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
        window->init("Child window plugged!", wui::rect{ 20, 30, 190, 190 }, wui::window_style::pinned, [this]() {
            if (createPluggedButton.lock())
                createPluggedButton.lock()->enable();
        });
    }

    PluggedWindow(std::shared_ptr<wui::window> &parentWindow_)
        : parentWindow(parentWindow_),
        window(new wui::window()),
        plugButton(new wui::button("Plug Window", std::bind(&PluggedWindow::Plug, this))),
        unplugButton(new wui::button("Unplug Window", std::bind(&PluggedWindow::Unplug, this))),
        createPluggedButton(),
        plugged(true)
    {
        window->add_control(unplugButton, wui::rect{ 10, 40, 110, 65 });
        window->add_control(plugButton, wui::rect{ 10, 85, 110, 110 });

        window->set_pin_callback([this](std::string &tooltip_text) {
            if (plugged)
            {
                Unplug();
                tooltip_text = "Pin the window";
            }
            else
            {
                Plug();
                tooltip_text = "Unpin the window";
            }
        });

        plugButton->disable();

        if (parentWindow.lock())
            parentWindow.lock()->add_control(window, wui::rect{ 20, 30, 190, 190 });

        Init();
    }
};

std::shared_ptr<wui::i_theme> MakeRedButtonTheme()
{
    auto redButtonTheme = wui::make_custom_theme();

    redButtonTheme->load_theme(*wui::get_default_theme());

    redButtonTheme->set_color(wui::button::tc, wui::button::tv_calm, wui::make_color(205, 15, 20));
    redButtonTheme->set_color(wui::button::tc, wui::button::tv_active, wui::make_color(235, 15, 20));
    redButtonTheme->set_color(wui::button::tc, wui::button::tv_border, wui::make_color(200, 215, 200));
    redButtonTheme->set_color(wui::button::tc, wui::button::tv_focused_border, wui::make_color(20, 215, 20));
    redButtonTheme->set_color(wui::button::tc, wui::button::tv_text, wui::make_color(190, 205, 190));
    redButtonTheme->set_color(wui::button::tc, wui::button::tv_disabled, wui::make_color(180, 190, 180));
    redButtonTheme->set_string(wui::image::tc, wui::image::tv_path, "IMAGES_DARK");
    
    return redButtonTheme;
}

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
    const std::string IDB_ACCOUNT = "";

    if (setlocale(LC_ALL,"") == NULL)
    {
        fprintf(stderr, "warning: could not set default locale\n");
    }

#endif
    bool runned = true;

    wui::set_default_theme_from_json("dark", dark_json);

    std::shared_ptr<wui::window> window(new wui::window());

    std::shared_ptr<wui::image> accountImage(new wui::image(IDB_ACCOUNT));
    window->add_control(accountImage, wui::rect{ 250, 100, 314, 164 });

    std::shared_ptr<PluggedWindow> pluggedWindow(new PluggedWindow(window));
    std::shared_ptr<wui::button> createPluggedButton(new wui::button("Create plugged window", []() {}));
    createPluggedButton->set_callback([&window, &pluggedWindow, &createPluggedButton]() {
        pluggedWindow.reset();
        pluggedWindow = std::shared_ptr<PluggedWindow>(new PluggedWindow(window));
        pluggedWindow->SetPluggedButton(createPluggedButton);
        createPluggedButton->disable(); });
    createPluggedButton->disable();
    pluggedWindow->SetPluggedButton(createPluggedButton);

    window->add_control(createPluggedButton, wui::rect{ 270, 50, 380, 75 });
    
    std::shared_ptr<wui::input> nameInput(new wui::input());
    //nameInput->set_text(L"Hello world!");
    window->add_control(nameInput, wui::rect{ 10, 250, 400, 275 });

    std::shared_ptr<wui::window> dialog(new wui::window());

    std::shared_ptr<wui::button> okButton(new wui::button("OK", [window, &dialog]()
    {
        window->block();

        std::shared_ptr<wui::button> dialogButton(new wui::button("Close", [&dialog]() { dialog->destroy(); }));
        dialog->add_control(dialogButton, wui::rect{ 10, 200, 100, 235 });

        dialog->init("Modal dialog", wui::rect{ 50, 50, 350, 350 }, wui::window_style::dialog, [window, &dialog]() { window->unlock(); /*dialog.reset();*/ });
    }));

    std::shared_ptr<wui::button> cancelButton(new wui::button("Cancel", [window]() { window->destroy(); }, wui::button_view::only_image, IDB_ACCOUNT, 24, MakeRedButtonTheme()));

    std::shared_ptr<wui::button> darkThemeButton(new wui::button("Set the dark theme", [&window, &pluggedWindow, &dialog, &cancelButton]()
    {
        wui::set_default_theme_from_json("dark", dark_json);
        window->update_theme();
        pluggedWindow->window->update_theme();
        dialog->update_theme(); 
        cancelButton->update_theme(MakeRedButtonTheme());
    }));
    window->add_control(darkThemeButton, wui::rect{ 140, 350, 260, 375 });
	
    std::shared_ptr<wui::button> whiteThemeButton(new wui::button("Set the white theme", [&window, &pluggedWindow, &dialog, &cancelButton]()
    {
        wui::set_default_theme_from_json("white", white_json);
        window->update_theme();
        pluggedWindow->window->update_theme();
        dialog->update_theme();
        cancelButton->update_theme(MakeRedButtonTheme());
    }));
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

    window->init("Welcome to WUI! ✌️", wui::rect{ 100, 100, 600, 600 }, wui::window_style::frame, [&runned]() {
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
