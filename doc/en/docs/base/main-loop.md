## Main application loop

In case of running on Windows, the standard infinite loop is started:

    #ifdef _WIN32
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;

Each running, non-child window becomes a message recipient via its existing wnd_proc. Then, depending on the type of event, either controls are redrawn, window position/size is handled, or the event is sent to subscribers. The lifetime of the first window created determines the lifetime of the application.

On linux the picture is slightly different, but it looks similar for the user and controls. Each non-child window, starts a separate thread to wait for events in xcb_wait_for_event() and sends them out to subscribers as they arrive. 

    bool window::init(...)
    {
        ...
        thread = std::thread(std::bind(&window::process_events, this));
    }

    void window::process_events()
    {
        xcb_generic_event_t *e = nullptr;
        while (runned && (e = xcb_wait_for_event(context_.connection)))
        {
            switch (e->response_type & ~0x80)
            {
                case XCB_EXPOSE:

                ...

All this code is hidden in ``window`` and ``framework``. ``framework`` has only 3 main functions ``init()``, ``run()`` and ``stop()``. ``init()`` should be called on the first line of ``main()``, ``run()`` after ``window->init(...)``, and ``stop()`` when the process needs to be terminated (e.g. the user pressed the "cross").

    int main(..)
    {
        wui::framework::init();

        MainFrame mainFrame;
        mainFrame.Run();

        wui::framework::run();

        return 0;
    }

Here ``wui::framework::end();`` is called in the main window closing callback:

    void MainFrame::Run()
    {
        window->init(wui::locale("main_frame", "caption"), { -1, -1, width, height },
            wui::window_style::frame, [this]() { 
                wui::framework::stop(); 
        });
    }

