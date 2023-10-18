## Dependencies

WUI uses three libraries in the thirdparty. These are: boost::nowide, nlohman::json and utf8 from Nemanja Trifunovic. The last two are header only and are not troublesome. boost::widen comes as a "cut" from boost, there are builds on vs 2017 and 2019 version of boost: 1.82. If your project already uses boost (especially a different version), it is better to specify for wui the path to your boost.

External dependencies are not available on Windows. On Linux, currently, xcb and cairo are required to work.
