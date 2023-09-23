# Basic interfaces 

The framework is based on several basic interfaces, such as i_window, i_control.
These interfaces contain methods that must be implemented by the window (in the case of i_window) and all controls (in the case of i_control). But each control (as well as the window implementation) also has additional specific methods.

# Window
This interface is implemented by the window

	class i_window
	{
	public:
    	virtual bool init(const std::string &caption, const rect &position, window_style style, std::function<void(void)> close_callback) = 0;
    	virtual void destroy() = 0;

    	virtual void add_control(std::shared_ptr<i_control> control, const rect &position) = 0;
    	virtual void remove_control(std::shared_ptr<i_control> control) = 0;
    
    	virtual void bring_to_front(std::shared_ptr<i_control> control) = 0;
    	virtual void move_to_back(std::shared_ptr<i_control> control) = 0;

    	virtual void redraw(const rect &position, bool clear = false) = 0;

    	virtual std::string subscribe(std::function<void(const event&)> receive_callback, event_type event_types, std::shared_ptr<i_control> control = nullptr) = 0;
    	virtual void unsubscribe(const std::string &subscriber_id) = 0;

    	virtual system_context &context() = 0;

	protected:
    	~i_window() {}
	};

## init
Initial window creation. This method should be called to create a separate (base) window, and can also be used to set child window parameters.

- caption - window title
- position - window dimensions and position. If left is set to -1 the window is centered horizontally, if top is set to -1 the window is centered vertically.
- style - combination of bit masks defining the appearance of the window
- close_callback - function called when the user closes the window, when ```destroy()`` is called or when a child window is detached from the base window (by calling ``remove_control()``).

## destroy
Called when the window is to be destroyed

## add_control
Adds a child control (including a child window) to the window.

- control - pointer to the control to be added
- position - position of the control on the window

## remove_control
Removes a child control from the window. If the child control is a window, it becomes independent (base).

- control - pointer to the control to be removed

## redraw
Starts redrawing of a part of the window with the existing controls in this area.

- position - boundaries of the redrawn area
- clear should be set for example when deleting or moving a control.

## subscribe
Used to receive events from the window. The method returns a unique subscriber identifier which should be passed to ``unsubscribe()`` for unsubscribing.

- receive_callback - function that receives events
- event_types - bit mask indicating which events should be received
- control - if this field is set, only mouse events occurring over this control and keyboard events will be received if it has input focus.
Otherwise, all events from the window will be received.

## unsubscribe
Used to unsubscribe from window events

- subscriber_id - unique identifier of the subscriber given by the ```subscribe`` method.

## system_context
Returns a reference to a structure containing platform-dependent entities. For example HWND window descriptor in Windows or xcb_connection in Linux.

# Control
This interface must be implemented by all controls, as well as window, because it can also be a control (child window).

	class i_control
	{
	public:
    	virtual void draw(graphic &gr, const rect &paint_rect) = 0;

    	virtual void set_position(const rect &position, bool redraw = true) = 0;
    	virtual rect position() const = 0;

    	virtual void set_parent(std::shared_ptr<window> window_) = 0;
    	virtual std::weak_ptr<window> parent() const = 0;
    	virtual void clear_parent() = 0;

    	virtual void set_topmost(bool yes) = 0;
    	virtual bool topmost() const = 0;

    	virtual void update_theme_control_name(const std::string &theme_control_name) = 0;
    	virtual void update_theme(std::shared_ptr<i_theme> theme_ = nullptr) = 0;

    	virtual void show() = 0;
    	virtual void hide() = 0;
    	virtual bool showed() const = 0;

    	virtual void enable() = 0;
    	virtual void disable() = 0;
    	virtual bool enabled() const = 0;

    	virtual bool focused() const = 0;  /// Returns true if the control is focused
    	virtual bool focusing() const = 0; /// Returns true if the control receives focus

    	friend class window;

	protected:
    	~i_control() {}

	};

## draw
The method is called by the window when it is necessary to redraw a control (for example, when calling window::redraw).

- gr - reference to the graphical context of the window, on which the control should draw itself.
- paint_rect - area to be redrawn. Usually ignored by simple controls

## set_position
Method to change the position of the control on the window. Coordinates are passed to this method relative to the control's parent window.

- position - new position of the control
- redraw - tells the window whether to clear the previous position of the control. 
For better performance, it should be set to false when ``set_position`` is called in response to a window resizing (since the entire window is cleared in this case).

## position
Returns the coordinates of the control relative to the base window. That is, if the control is on a child window, ````position()```` should return the position of the control on the base window. 
of the control on the base (physical) window.

## set_parent
The method called by the parent window when ```add_control()`` is called allows the control to get a pointer to its parent window.

## parent
Returns a pointer to the window containing the control

## clear_parent
The method called by the parent window when ```remove_control()`` is called clears a pointer to the control's parent window.

## topmost
Tells the parent window whether to draw the control on top of all other controls.

## update_theme
Changes the visual theme of the control. If the parameter is nullptr, the default application theme is used.

## show
Turns on the control's display (if it was turned off by the ``hide()`` method).

## hide
Disables control display

## show
Returns the control's display state (enabled or disabled)

## enable
Enables the control (if it was disabled by the ```disable()`` method)

## disable
Disables the control. A disabled control, as a rule, does not react to user events, does not accept input focus and is grayed out.

## focused
Returns whether the control has input focus.

## focused
Returns whether the control accepts input focus. Disabled control, control not interacting with user returns false