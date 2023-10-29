## Multithreading issues

WUI does not use a single mutex. Control callbacks and system events come only from a single thread on Windows (proc_wnd) or from a window thread waiting for xcb_wait_for_event(). 

It is recommended to perform all UI manipulations either in callbacks / received system events, or in one special UI track of the application. 

If you plan to window.add_control() / window.remove_control() from different tracks, it is necessary to implement protection at the application code level.
