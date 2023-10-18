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

On linux the picture is slightly different, but it looks similar for the user and controls. Each non-child window, starts a separate thread to wait for events in xcb_wait_for_event() and sends them out to subscribers as they arrive. Accordingly, in order for an application to wait for a main window to close, for example, that window must provide a method that returns true until the user or the application itself closes the window.

    #elif __linux__
    // Wait for main window
    while (mainFrame.Runned())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
    #endif