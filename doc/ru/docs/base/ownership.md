# Плоская модель владения контролами

Один из ключевых принципов работы с WUI — **плоская модель владения контролами**. Эта модель упрощает управление жизненным циклом окон и контролов, делая код более понятным и предсказуемым.

## Архитектурный принцип: Flat Ownership (Плоское владение)

В libwui используется модель, где UI-компоненты являются соседями (siblings) внутри логического контроллера (Диалога или Фрейма), а не выстраиваются в иерархию владения.

### Ключевые особенности

**Отсутствие циклических ссылок**  
Контролы не владеют друг другом. Все контролы владеются одним объектом-контейнером (обычно классом диалога).

**RAII**  
Жизненный цикл контролов привязан к жизненному циклу владеющего объекта. Пока жив объект логики — живы и интерфейсные элементы.

**Window не владеет контролами**  
Метод `window->add_control(button, rect)` не передает владение окну. Окно хранит невладеющие ссылки на контролы для отрисовки и проброса событий.

**Возможность перемещения контролов**  
Контрол можно переместить из одного окна в другое, просто вызвав `add_control` в новом окне, без изменения структуры владения.

### Сравнение с другими подходами

В отличие от Qt с иерархическим владением через `QObject::parent`, где удаление родителя автоматически удаляет детей, в libwui владение явное и контролируется разработчиком.

### Пример

```cpp
void DialingDialog::Cancel() {
    window->destroy(); // Окно уничтожено
    // cancelButton всё ещё доступен в памяти DialingDialog
    // можно прочитать его состояние или выполнить дополнительную логику
}
```

## Основной принцип

Все контролы окна владеются **одним объектом-родителем** (обычно это класс диалога или окна), который хранит их в виде `std::shared_ptr`. При уничтожении родителя все контролы автоматически освобождаются.

### Преимущества плоской модели

- **Простота управления памятью** — не нужно вручную удалять контролы
- **Предсказуемый жизненный цикл** — все контролы создаются и уничтожаются вместе с окном
- **Отсутствие циклических ссылок** — контролы не владеют друг другом
- **Безопасность** — используется `std::shared_ptr`/`std::weak_ptr` для управления временем жизни

## Паттерн использования

Рассмотрим типичный паттерн на примере диалога:

```cpp
class DialingDialog
{
public:
    DialingDialog(std::weak_ptr<wui::window> transientWindow, 
                  std::function<void()> cancelCallback);
    ~DialingDialog();
    
    void Run(std::string_view subscriber);
    void End();

private:
    static const int32_t WND_WIDTH = 300;
    static const int32_t WND_HEIGHT = 150;
    
    std::weak_ptr<wui::window> transientWindow;
    std::function<void()> cancelCallback;
    
    // Плоское владение контролами
    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::text> text;
    std::shared_ptr<wui::image> image;
    std::shared_ptr<wui::button> cancelButton;
    
    void Cancel();
};
```

## Создание контролов

Контролы создаются в методе `Run()` и сразу добавляются на окно:

```cpp
void DialingDialog::Run(std::string_view subscriber)
{
    window->set_transient_for(transientWindow.lock());
    
    // Создание контролов
    text = std::make_shared<wui::text>();
    image = std::make_shared<wui::image>(IMG_CALLOUT);
    cancelButton = std::make_shared<wui::button>(
        wui::locale("button", "cancel"), 
        std::bind(&DialingDialog::Cancel, this)
    );
    
    // Добавление контролов на окно
    window->add_control(text, { 84, 10, WND_WIDTH - 10, 65 });
    window->add_control(image, { 10, 10, 74, 74 });
    window->add_control(cancelButton, { 
        WND_WIDTH - 110, WND_HEIGHT - 40, 
        WND_WIDTH - 10, WND_HEIGHT - 10 
    });
    
    window->set_default_push_control(cancelButton);
    
    // Инициализация окна с коллбэком закрытия
    window->init("", { -1, -1, WND_WIDTH, WND_HEIGHT }, 
                 wui::window_style::border_all, 
                 [this]() {
                     // Освобождение контролов при закрытии окна
                     cancelButton.reset();
                     image.reset();
                     text.reset();
                 });
    
    text->set_text(wui::locale("dialing_dialog", "dial") + 
                   " " + std::string(subscriber));
}
```

## Уничтожение контролов

Контролы освобождаются в коллбэке закрытия окна, который передается в `init()`:

```cpp
window->init("", { -1, -1, WND_WIDTH, WND_HEIGHT }, 
             wui::window_style::border_all, 
             [this]() {
                 cancelButton.reset();
                 image.reset();
                 text.reset();
             });
```

Это гарантирует, что все контролы будут корректно уничтожены при закрытии окна.

## Методы завершения диалога

```cpp
void DialingDialog::End()
{
    if (window) {
        window->destroy();
    }
}

void DialingDialog::Cancel()
{
    if (window) {
        window->destroy();
    }
    cancelCallback();
}
```

## Важные замечания

1. **Не используйте вложенное владение** — контролы не должны владеть друг другом
2. **Используйте `std::weak_ptr`** для временных ссылок на окна (например, `transientWindow`)
3. **Всегда освобождайте контролы** в коллбэке закрытия окна
4. **Проверяйте валидность** перед использованием (`if (window)`)

## Типичные ошибки

### ❌ Неправильно: контрол владеет другим контролом

```cpp
// panel хранит button внутри себя как shared_ptr
class BadDialog {
    std::shared_ptr<wui::panel> panel;
    // panel->add_control(button) создаёт владение внутри panel
    // При уничтожении dialog -> panel -> button возникает проблема
};
```

Проблема: если `panel` владеет `button` внутри себя, а `dialog` тоже владеет `button` — возникает неопределённое поведение при уничтожении.

### ✅ Правильно: все контролы владеются одним объектом

```cpp
class GoodDialog {
    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::button> button;
    std::shared_ptr<wui::panel> panel;
    
    GoodDialog() {
        window = std::make_shared<wui::window>();
        button = std::make_shared<wui::button>("Click", [](){});
        panel = std::make_shared<wui::panel>();
        
        // add_control не забирает владение, только ссылку
        window->add_control(button, {10, 10, 100, 30});
        window->add_control(panel, {0, 0, 200, 200});
    }
};
```

Все контролы владеются `GoodDialog`, окно только ссылается на них.

## Заключение

Плоская модель владения контролами — это простой и надежный способ управления жизненным циклом UI-элементов в WUI. Следование этому паттерну помогает избежать утечек памяти, циклических ссылок и неопределенного поведения.
