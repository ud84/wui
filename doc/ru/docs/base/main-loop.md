## Главный цикл приложения

В случае работы на Windows запускается стандартный бесконечный цикл:

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

Каждое запущенное, не дочернее окно, становится получателем сообщений через имеющейся в нем wnd_proc. Далее, в зависимости от типа события, производится либо перерисовка контролов, работа с положением/размером окна, либо событие посылается подписчикам. Срок жизни первого созданного окна определяет срок жизни приложения.

На линукс картина слегка отличается, но для пользователя и контролов выглядит аналогично. Каждое не дочернее окно, запускает отдельный тред для ожидания событий в xcb_wait_for_event() и по мере их поступления рассылает их подписчикам. 

    bool window::init(...)
    {
        ...
        thread = std::thread(std::bind(&window::process_events, this));
    }

    void window::process_events()
    {
        xcb_generic_event_t *e = nullptr;
        while (started && (e = xcb_wait_for_event(context_.connection)))
        {
            switch (e->response_type & ~0x80)
            {
                case XCB_EXPOSE:

                ...

Весь этот код скрыт в ``window`` и ``framework``.
framework имеет всего 3 главные функции ``init()``, ``run()`` и ``stop()``. ``init()`` нужно вызвать в первой строке ``main()``, ``run()`` после ``window->init(...)``, а ``stop()`` когда нужно завершить процесс (например пользователь нажал "крестик").

    int main(..)
    {
        wui::framework::init();

        MainFrame mainFrame;
        mainFrame.Run();

        wui::framework::run();

        return 0;
    }

Здесь ``wui::framework::end();`` вызывается в коллбеке закрытия главного окна:

    void MainFrame::Run()
    {
        window->init(wui::locale("main_frame", "caption"), { -1, -1, width, height },
            wui::window_style::frame, [this]() { 
                wui::framework::stop(); 
        });
    }

