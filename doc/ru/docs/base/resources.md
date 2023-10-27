# Ресурсы

Для удобного и единообразного отображения множества контролов и надписей приложения, удобства работы не программистов, например дизайнеров, переводчиков реализованы подсистемы theme и locale. 

Приложение всегда имеет текущую тему и локаль. Это по сути конфиги, выдающие значения по паре секция + ключ. В каждый контрол (и окно, так как оно тоже контрол) передается тема для получения этим контролом значений своих цветов, размеров, толщин, шрифтов и прочего. По умолчанию, в контрол можно не передавать кастомную тему, тогда он будет использовать общую текущую тему приложения.

У пользовательского кода для формирования надписей есть доступ к текущей локали приложения. Это позволяет собрать все текстовые ресурсы в одном месте и также, в одном месте, менять их для всего приложения.

Приложение имеет возможность менять текущую тему и локаль, что вызывает автоматическую смену внешнего вида контролов и окон / языка всего приложения.

Технически подсистемы реализованы схоже, рассмотрим на примере theme
Тема представляет из себя json содержащий значения параметров для контролов, например для окна и надписи и изображений.

## Тема dark:
    {
        "controls": [
        {
            "type": "window",
            "background": "#131519",
            "border": "#404040",
            "border_width": 1,
            "text": "#f5f5f0",
            "active_button": "#3b3d41",
            "caption_font": {
                "name": "Segoe UI",
                "size": 18,
                "decorations": "normal"
            }
        },
        {
            "type": "text",
            "color": "#f5f5f0",
            "font": {
                "name": "Segoe UI",
                "size": 18
            }
        },
        {
            "type": "image",
            "resource": "IMAGES_DARK",
            "path": "~/.hello_wui/res/images/dark"
        }
        /* И так далее */
    }

## Тема light:

    {
        "controls": [
        {
            "type": "window",
            "background": "#fffffe",
            "border": "#9a9a9a",
            "border_width": 1,
            "text": "#191914",
            "caption_font": {
                "name": "Segoe UI",
                "size": 18
            }
        },
        {
            "type": "text",
            "color": "#191914",
            "font": {
                "name": "Segoe UI",
                "size": 18
            }
        },
        {
            "type": "image",
            "resource": "IMAGES_LIGHT",
            "path": "~/.hello_wui/res/images/light"
        }
        /* И так далее */
    }

Данный подход предоставляет приложению и контролам прозрачный,  централизованный механизм управления отображением. При необходимости создать кастомный контрол (например красную кнопку) можно просто добавить в json новый раздел: 

    {
        "type": "red_button",
        "calm": "#c61818",
        "active": "#e31010",
        "border": "#c90000",
        "border_width": 1,
        "focused_border": "#dcd2dc",
        "text": "#f0f1f1",
        "disabled": "#a5a5a0",
        "round": 0,
        "focusing": 1,
        "font": {
            "name": "Segoe UI",
            "size": 18
        }
    }

А при создании контрола указать имя контрола: “red_button”, например:

    cancelButton(new wui::button(wui::locale("button", "cancel"), [this]() { window->destroy(); }, "red_button"))

Для работы с пиктограммами и подобными изображениями используется контрол image. Он также использует theme для получения идентификатора win32 ресурса или пути к файлу изображения. Это позволяет создать изображение 

    logoImage(new wui::image(IMG_LOGO)) 

где:

    #ifdef _WIN32
    #define IMG_LOGO 4010
    #else
    static constexpr const char* IMG_LOGO = "logo.png";
    #endif

Логотип будет загружен в соответствии с заданной темой.