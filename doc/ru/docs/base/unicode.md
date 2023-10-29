## Unicode

Используется только UTF-8 передаваемый в обычных std::string / char *. 

Для взаимодействия с WinAPI которой нужен utf16 в wchar, используется boost::nowide::widen() / boost::nowide::narrow(). boost::nowide не имеет зависимостей от boost и поставляется вместе с WUI в thirdparty. Таким образом, если в вашем проекте нет boost вам не придется включать его в зависимости для WUI. 

Приложение также должно использовать boost::nowide для работы WUI совместно с WinAPI.

Подробнее о том, почему wchar не нужен, написано здесь: [https://utf8everywhere.org/](https://utf8everywhere.org/)

На Linux boost::nowide не требуется, и зависимость от него исключается. 
