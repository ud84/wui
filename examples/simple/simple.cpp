// wui.cpp : Defines the entry point for the application.
//

#include <wui/theme/theme.hpp>
#include <wui/locale/locale.hpp>
#include <wui/window/window.hpp>
#include <wui/control/button.hpp>
#include <wui/control/input.hpp>
#include <wui/control/menu.hpp>
#include <wui/control/list.hpp>
#include <wui/control/select.hpp>
#include <wui/control/image.hpp>
#include <wui/control/text.hpp>
#include <wui/control/message.hpp>
#include <wui/control/splitter.hpp>
#include <wui/control/progress.hpp>
#include <wui/control/slider.hpp>
#include <wui/control/panel.hpp>

#ifdef _WIN32
#include <Resource.h>
#include <gdiplus.h>
#endif

#ifndef _WIN32
const std::string IMG_ACCOUNT = "account.png";
const std::string IMG_SETTINGS = "settings.png";
#endif

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
    //redButtonTheme->set_string(wui::image::tc, wui::image::tv_path, "IMAGES_DARK");

    return redButtonTheme;
}

std::shared_ptr<wui::i_theme> MakeToolButtonTheme()
{
    auto toolButtonTheme = wui::make_custom_theme();

    toolButtonTheme->load_theme(*wui::get_default_theme());

    auto background_color = wui::theme_color(wui::window::tc, wui::window::tv_background);

    toolButtonTheme->set_color(wui::button::tc, wui::button::tv_calm, background_color);
    toolButtonTheme->set_color(wui::button::tc, wui::button::tv_active, wui::theme_color(wui::window::tc, wui::window::tv_active_button));
    toolButtonTheme->set_color(wui::button::tc, wui::button::tv_disabled, background_color);
    toolButtonTheme->set_dimension(wui::button::tc, wui::button::tv_round, 0);
    toolButtonTheme->set_dimension(wui::button::tc, wui::button::tv_border_width, 0);

    return toolButtonTheme;
}

struct PluggedWindow : public std::enable_shared_from_this<PluggedWindow>
{
    std::weak_ptr<wui::window> parentWindow;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::list> list;
    std::shared_ptr<wui::menu> popupMenu;
    std::shared_ptr<wui::panel> panel;
    std::shared_ptr<wui::button> button1, button2, button3;
    std::shared_ptr<wui::input> input;
    std::shared_ptr<wui::message> messageBox;
    std::shared_ptr<wui::window> dialog;
    std::weak_ptr<wui::button> creationButton;

    bool plugged;

    void Plug()
    {
        auto parentWindow_ = parentWindow.lock();
        if (parentWindow_)
        {
            parentWindow_->add_control(window, { 0, 30, 300, parentWindow_->position().height() });
        }

        plugged = !plugged;
    }

    void Unplug()
    {
        if (parentWindow.lock())
            parentWindow.lock()->remove_control(window);

        Init();
		
        plugged = !plugged;
    }

    void SetCreationButton(std::shared_ptr<wui::button> &creationButton_)
    {
        creationButton = creationButton_;
    }

    void Init()
    {
        window->init("Child window plugged!", { 0 }, 
            static_cast<wui::window_style>(static_cast<uint32_t>(wui::window_style::pinned) | static_cast<uint32_t>(wui::window_style::border_right)),
            [this]() {
            if (creationButton.lock())
                creationButton.lock()->enable();
        });
    }

    PluggedWindow(std::shared_ptr<wui::window> &parentWindow_)
        : parentWindow(parentWindow_),
        window(new wui::window()),
        list(new wui::list()),
        popupMenu(new wui::menu()),
        panel(new wui::panel()),
		button1(new wui::button("Button 1", [this]() { 
            messageBox->show("Lorem Ipsum is simply dummy text of the printing and typesetting industry.\nLorem Ipsum has been the industry's\nstandard dummy text ever since the 1500s, when an unknown printer took\na galley of type and scrambled it to make a type specimen book.",
                "hello world", wui::message_icon::information, wui::message_button::ok, [](wui::message_result) {});
        }, wui::button_view::image, IMG_ACCOUNT, 16)),
        button2(new wui::button("Button 2", [this]() {
            window->emit_event(310, 200);
        }, wui::button_view::image, IMG_ACCOUNT, 16)),
        button3(new wui::button("Button 3", []() {}, wui::button_view::image, IMG_ACCOUNT, 16)),
        input(new wui::input("", wui::input_view::password)),
        messageBox(new wui::message(window, true)),
        dialog(new wui::window()),
        creationButton(),
        plugged(false)
    {
        button1->disable_focusing();
        button2->disable_focusing();
        button3->disable_focusing();

        list->set_draw_callback(std::bind(&PluggedWindow::DrawListItem, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

        list->set_item_right_click_callback([this](int32_t item, int32_t x, int32_t y) { popupMenu->show_on_point(x, y); });

        list->update_columns({ { 30, "##" }, { 100, "Name" }, { 100, "Role" } });
        
        list->set_item_height(32);
        list->set_item_count(100);

        popupMenu->set_items({
            { 0, wui::menu_item_state::normal, "Bla bla bla", "", nullptr, {}, [](int32_t i) {} },
            { 1, wui::menu_item_state::separator, "Other", "", nullptr, {}, [](int32_t i) {} },
            { 2, wui::menu_item_state::normal, "Another", "", nullptr, {}, [](int32_t i) {} }
            });

        window->add_control(popupMenu, { 0 });

        window->add_control(list, { 0 });
        window->add_control(panel, { 0 });
        window->add_control(button1, { 0 });
        window->add_control(button2, { 0 });
        window->add_control(button3, { 0 });
        window->add_control(input, { 0 });

        window->set_pin_callback([this](std::string &tooltip_text) {
            if (plugged)
            {
                Unplug();
                tooltip_text = wui::locale("window", "pin");
            }
            else
            {
                Plug();
                tooltip_text = wui::locale("window", "unpin");
            }
        });

        window->subscribe([this](const wui::event &e) {
            if (e.internal_event_.type == wui::internal_event_type::size_changed)
            {
                int32_t w = e.internal_event_.x, h = e.internal_event_.y;

                list->set_position({ 10, 30, w - 10, h - 40 }, false);
                panel->set_position({ 0, h - 35, w, h }, false);
                button1->set_position({ 10, h - 30, 30, h - 10 }, false);
                button2->set_position({ 40, h - 30, 60, h - 10 }, false);
                button3->set_position({ 70, h - 30, 90, h - 10 }, false);
                input->set_position({ 100, h - 30, w - 10, h - 10 }, false);
            }
            else if (e.internal_event_.type == wui::internal_event_type::user_emitted)
            {
                int32_t x = e.internal_event_.x, y = e.internal_event_.y;

                messageBox->show("user emitted event received, x: " + std::to_string(x) + ", y: " + std::to_string(y),
                    "user emitted event", wui::message_icon::information, wui::message_button::yes_no, [this](wui::message_result result) {
                    if (result == wui::message_result::yes)
                    {
                        dialog->set_transient_for(window);
                        dialog->init("Modal dialog", { 50, 50, 350, 350 }, wui::window_style::dialog, []() {});
                    }
                });
            }
        }, wui::event_type::internal);

        Plug();
        Init();
    }

    void DrawListItem(wui::graphic &gr, int32_t nItem, const wui::rect &itemRect_, wui::list::item_state state, const std::vector<wui::list::column> &columns)
    {
        auto border_width = wui::theme_dimension(wui::list::tc, wui::list::tv_border_width);

        auto itemRect = itemRect_;

        if (itemRect.bottom > list->position().bottom - border_width)
        {
            itemRect.bottom = list->position().bottom - border_width;
        }

        if (state == wui::list::item_state::active)
        {
            gr.draw_rect(itemRect, wui::theme_color(wui::list::tc, wui::list::tv_active_item));
        }
        else if (state == wui::list::item_state::selected)
        {
            gr.draw_rect(itemRect, wui::theme_color(wui::list::tc, wui::list::tv_selected_item));
        }

        auto textColor = wui::theme_color(wui::input::tc, wui::input::tv_text);
        auto font = wui::theme_font(wui::list::tc, wui::list::tv_font);
        
        auto textHeight = gr.measure_text("Qq", font).height();
        if (textHeight <= itemRect.height())
        {
            auto textRect = itemRect_;

            textRect.move(20, (itemRect_.height() - textHeight) / 2);

            gr.draw_text(textRect, "Item " + std::to_string(nItem), textColor, font);
        }
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
    if (setlocale(LC_ALL,"") == NULL)
    {
        fprintf(stderr, "warning: could not set default locale\n");
    }

#endif
    bool runned = true;

#ifdef _WIN32
    wui::set_default_theme_from_resource("dark", TXT_DARK_THEME, "JSONS");
    wui::set_locale_from_resource("en", TXT_LOCALE_EN, "JSONS");
#elif __linux__
    wui::set_default_theme_from_file("dark", "dark.json");
    wui::set_locale_from_file("en", "en_locale.json");
#endif

    std::shared_ptr<wui::window> window(new wui::window());

    std::shared_ptr<wui::image> menuImage1(new wui::image(IMG_ACCOUNT));
    std::shared_ptr<wui::image> menuImage2(new wui::image(IMG_SETTINGS));

    std::shared_ptr<wui::menu> menu(new wui::menu());

    menu->set_items({
            { 0, wui::menu_item_state::separator, "Bla bla bla", "", menuImage1, {}, [](int32_t i) {} },
            { 1, wui::menu_item_state::normal, "Expand me 1", "", nullptr, {
                    { 11, wui::menu_item_state::normal, "Expanded 1.1", "", nullptr, {}, [](int32_t i) {} },
                    { 12, wui::menu_item_state::normal, "Expanded 1.2", "", nullptr, {
                            { 121, wui::menu_item_state::normal, "Expanded 1.1.1", "", nullptr, {}, [](int32_t i) {} },
                            { 122, wui::menu_item_state::normal, "Expanded 1.1.2", "Shift+Del", menuImage2, {}, [](int32_t i) {} },
                            { 123, wui::menu_item_state::separator, "Expanded 1.1.3", "", nullptr, {}, [](int32_t i) {} },
                        }, [](int32_t i) {} },
                    { 13, wui::menu_item_state::normal, "Expanded 1.3", "", nullptr, {}, [](int32_t i) {} },
                }, [](int32_t i) {} },
            { 2, wui::menu_item_state::separator, "Expand me 2", "Ctrl+Z", nullptr, {
                    { 21, wui::menu_item_state::normal, "Expanded 2.1", "", nullptr, {}, [](int32_t i) {} },
                    { 22, wui::menu_item_state::normal, "Expanded 2.2", "", nullptr, {}, [](int32_t i) {} },
                    { 23, wui::menu_item_state::separator, "Expanded 2.3", "", nullptr, {}, [](int32_t i) {} },
                }, [](int32_t i) {} },
            { 3, wui::menu_item_state::normal, "Exit", "Alt+F4", nullptr, {}, [&window](int32_t i) { window->destroy(); } }
        });

    window->add_control(menu, { 0 });

    std::shared_ptr<wui::button> menuButton(new wui::button("Settings", []() {}, wui::button_view::image, IMG_SETTINGS, 32, MakeToolButtonTheme()));
    menuButton->set_callback([&menu, &menuButton]() { menu->show_on_control(menuButton, 5); });
    menuButton->disable_focusing();
    window->add_control(menuButton, { 0 });

    std::shared_ptr<wui::progress> horizProgressBar(new wui::progress(0, 100, 50));
    window->add_control(horizProgressBar, { 450, 100, 650, 125 });

    std::shared_ptr<wui::slider> horizSlider(new wui::slider(0, 100, 50, [&horizProgressBar](int32_t value) { horizProgressBar->set_value(value); }, wui::slider_orientation::horizontal));
    window->add_control(horizSlider, { 450, 140, 650, 165 });

    std::shared_ptr<wui::progress> vertProgressBar(new wui::progress(0, 100, 80, wui::progress_orientation::vertical));
    window->add_control(vertProgressBar, { 700, 30, 725, 125 });

    std::shared_ptr<wui::slider> vertSlider(new wui::slider(0, 100, 80, [&vertProgressBar](int32_t value) { vertProgressBar->set_value(value); }, wui::slider_orientation::vertical));
    window->add_control(vertSlider, { 660, 30, 685, 155 });

    std::shared_ptr<wui::image> accountImage(new wui::image(IMG_ACCOUNT));
    window->add_control(accountImage, { 350, 100, 414, 164 });

    std::shared_ptr<PluggedWindow> pluggedWindow(new PluggedWindow(window));
    std::shared_ptr<wui::button> createPluggedButton(new wui::button("Create plugged window", []() { }));
    createPluggedButton->set_callback([&window, &pluggedWindow, &createPluggedButton]() {
        pluggedWindow.reset();
        pluggedWindow = std::shared_ptr<PluggedWindow>(new PluggedWindow(window));
        pluggedWindow->SetCreationButton(createPluggedButton);
        createPluggedButton->disable(); });
    createPluggedButton->disable();
    pluggedWindow->SetCreationButton(createPluggedButton);

    std::shared_ptr<wui::splitter> vertSplitter(new wui::splitter(wui::splitter_orientation::vertical, [&pluggedWindow](int32_t x, int32_t y) {
        if (pluggedWindow->plugged)
        {
            auto pos = pluggedWindow->window->position();
            pluggedWindow->window->set_position({ 0, pos.top, x, pos.bottom }, true);
        }
    }));
    window->add_control(vertSplitter, { 0 });

    window->add_control(createPluggedButton, { 320, 50, 340, 75 });

    std::shared_ptr<wui::text> text0(new wui::text("Lorem Ipsum is simply dummy text of the printing and typesetting industry.\nLorem Ipsum has been the industry's\nstandard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum."));
    window->add_control(text0, { 320, 180, 890, 240 });
    
    std::shared_ptr<wui::input> nameInput(new wui::input());
    nameInput->set_text("Hello world!");
    //nameInput->set_input_view(wui::input_view::readonly);
    window->add_control(nameInput, { 320, 250, 890, 275 });

    std::shared_ptr<wui::select> someSelect(new wui::select());
    someSelect->set_items({
            { 0, "Item 0" },
            { 1, "Item 1" },
            { 2, "Item 2" },
            { 3, "Item 3" },
            { 4, "Item 4" },
            { 5, "Item 5" }
        });
    window->add_control(someSelect, { 320, 300, 890, 325 });

    std::shared_ptr<wui::window> dialog(new wui::window());

    std::shared_ptr<wui::button> okButton(new wui::button("OK", [window, &dialog]()
    {
        std::shared_ptr<wui::button> dialogButton(new wui::button("Close", [&dialog]() { dialog->destroy(); }));
        dialog->add_control(dialogButton, { 10, 200, 100, 235 });

        dialog->set_transient_for(window);
        dialog->init("Modal dialog", { 50, 50, 350, 350 }, wui::window_style::dialog, [&dialog]() { /*dialog.reset();*/ });
    }));

    std::shared_ptr<wui::button> cancelButton(new wui::button("Cancel", [window]() { window->destroy(); }, wui::button_view::image_right_text, IMG_ACCOUNT, 24, MakeRedButtonTheme()));

    std::shared_ptr<wui::button> darkThemeButton(new wui::button("Set the dark theme", [&window, &pluggedWindow, &dialog, &cancelButton]()
    {
#ifdef _WIN32
        wui::set_default_theme_from_resource("dark", TXT_DARK_THEME, "JSONS");
#elif __linux__
        wui::set_default_theme_from_file("dark", "dark.json");
#endif

        window->update_theme();
        pluggedWindow->window->update_theme();
        dialog->update_theme(); 
        cancelButton->update_theme(MakeRedButtonTheme());
    }));
    window->add_control(darkThemeButton, { 320, 350, 440, 375 });
	
    std::shared_ptr<wui::button> whiteThemeButton(new wui::button("Set the white theme", [&window, &pluggedWindow, &dialog, &cancelButton]()
    {
#ifdef _WIN32
        wui::set_default_theme_from_resource("light", TXT_LIGHT_THEME, "JSONS");
#elif __linux__
        wui::set_default_theme_from_file("light", "light.json");
#endif

        window->update_theme();
        pluggedWindow->window->update_theme();
        dialog->update_theme();
        cancelButton->update_theme(MakeRedButtonTheme());
    }, wui::button_view::anchor));
    window->add_control(whiteThemeButton, { 460, 350, 580, 375 });

    window->add_control(okButton, { 240, 450, 350, 480 });
    window->add_control(cancelButton, { 370, 450, 480, 480 });

    window->set_min_size(100, 100);

    auto sid = window->subscribe([&menuButton, text0, &pluggedWindow, &vertSplitter, &nameInput, &someSelect, &okButton, &cancelButton](const wui::event &e) {
        if (e.internal_event_.type == wui::internal_event_type::size_changed)
        {
            int32_t w = e.internal_event_.x, h = e.internal_event_.y;

            if (pluggedWindow->plugged)
            {
                auto pos = pluggedWindow->window->position();
                pluggedWindow->window->set_position({ 0, 30, pos.width(), h }, false);
                vertSplitter->set_position({ pos.width(), 30, pos.width() + 5, h }, false);
            }

            menuButton->set_position({ w - 42, 50, w - 10, 82 }, false);
            text0->set_position({ 320, 180, w - 10, 240 }, false);
            nameInput->set_position({ 320, 250, w - 10, 275 }, false);
            someSelect->set_position({ 320, 300, w - 10, 325 }, false);
            okButton->set_position({ w - 250, h - 50, w - 150, h - 20 }, false);
            cancelButton->set_position({ w - 120, h - 50, w - 20, h - 20 }, false);
        }
    }, wui::event_type::internal);

    window->set_switch_theme_callback([&window, &pluggedWindow, &dialog, &cancelButton](std::string &tooltip_text) {
        auto theme_name = wui::get_default_theme()->get_name();

        if (theme_name == "dark")
        {
            tooltip_text = wui::locale("window", "dark_theme");
#ifdef _WIN32
            wui::set_default_theme_from_resource("light", TXT_LIGHT_THEME, "JSONS");
#elif __linux__
            wui::set_default_theme_from_file("light", "light.json");
#endif
        }
        else if (theme_name == "light")
        {
            tooltip_text = wui::locale("window", "light_theme");
#ifdef _WIN32
            wui::set_default_theme_from_resource("dark", TXT_DARK_THEME, "JSONS");
#elif __linux__
            wui::set_default_theme_from_file("dark", "dark.json");
#endif
        }

        window->update_theme();
        pluggedWindow->window->update_theme();
        dialog->update_theme();
        cancelButton->update_theme(MakeRedButtonTheme());
    });

    window->init("Welcome to WUI!", { -1, -1, 900, 600 }, 
        static_cast<wui::window_style>(static_cast<uint32_t>(wui::window_style::frame) |
            static_cast<uint32_t>(wui::window_style::switch_theme_button) |
            static_cast<uint32_t>(wui::window_style::border_all)),
        //wui::window_style::frame,
        [&runned]() {
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
