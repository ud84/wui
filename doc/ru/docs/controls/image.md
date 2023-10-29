# Изображение

image нужен для единообразного отображения пиктограмм с учетом визуальной темы. Например button использует image для рисования пиктограмм на себе. image рисует себя из ресурса, соответствующего визуальной теме. 

Пример использования image:
Создаем в конструкторе содержащего image класса

    logoImage(new wui::image(IMG_LOGO))...

IMG_LOGO определен в resourse.h приложения следующим образом:

    #ifdef _WIN32
    #define IMG_LOGO				  109
    #else // _WIN32
    static constexpr const char* IMG_LOGO = "logo.png";
    #endif

Таким образом, изображение будет взято из ресурса exe на Windows или из файла на других системах.
Магия смены изображения при смене темы реализована следующим образом. image имеет в theme свои настройки:

## light.json:

    {
        "type": "image",
        "resource": "IMAGES_LIGHT",
        "path": "res/images/light"
    }

## dark.json:

    {
        "type": "image",
        "resource": "IMAGES_DARK",
        "path": "res/images/dark"
    }

Путь к файлу ресурса составляется из пути указанном в theme и имени файла в image что приводит к автоматической замене всех изображений приложения при смене темы.
На Windows стоит упомянуть как организован rc файл приложения.

    IMG_LOGO IMAGES_DARK   "res\\images\\dark\\logo.png"
    IMG_LOGO IMAGES_LIGHT  "res\\images\\light\\logo.png"

Таким образом, замена группы IMAGES_DARK / IMAGES_LIGHT вызывает аналогичный эффект как с файлами, без необходимости менять ID ресурса.

