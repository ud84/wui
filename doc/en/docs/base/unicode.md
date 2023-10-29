## Unicode

Only UTF-8 passed in regular std::string / char * is used. 

To interact with WinAPI which needs utf16 in wchar, boost::nowide::widen() / boost::nowide::narrow() is used. boost::nowide has no dependencies on boost and comes with the WUI in thirdparty. Thus, if your project does not have boost you do not have to include it in the dependencies for the WUI. 

The application should also use boost::nowide to run WUI together with WinAPI.

More information on why wchar is not needed is written here: [https://utf8everywhere.org/](https://utf8everywhere.org/)

On Linux boost::nowide is not required, and dependency on it is eliminated.
