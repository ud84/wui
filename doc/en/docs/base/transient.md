## Transitivity

Applications are inconceivable without modal dialogs. To implement them, window has a method:

    void set_transient_for(std::shared_ptr<window> window_, bool docked = true);

This method indicates to the parent window that a certain window should be made modal relative to it. The docked flag specifies that the modal window should be displayed in the base window without creating a physical system window. If the modal window is larger than the parent window, this flag is ignored and a new system window is created. 

Strictly speaking, there is no modality in the usual WinAPI sense in the library. That is, the init() call of a transient window does not block the calling code, but it is bypassed by continuing the logic in the close_callback callback passed to init().
As practice has shown, it is quite possible to live with it, and for some things it even turns out to be more convenient.
