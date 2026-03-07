# Все контролы

Библиотека WUI предоставляет набор стандартных контролов для создания пользовательского интерфейса.

## Стандартные контролы

| Контрол | Описание |
|---------|----------|
| [Button](button.md) | Кнопка для взаимодействия с пользователем. Поддерживает текст, изображения, переключатели |
| [Image](image.md) | Отображение изображений с поддержкой визуальных тем |
| [Input](input.md) | Текстовое поле ввода. Однострочное, многострочное, пароль, только чтение |
| [List](list.md) | Список элементов с поддержкой колонок, прокрутки и отрисовки |
| [Menu](menu.md) | Контекстное меню с поддержкой вложенных пунктов и иконок |
| [Message](message.md) | Модальные диалоги для отображения сообщений пользователю |
| [Panel](panel.md) | Контейнер для группировки элементов или пользовательской отрисовки |
| [Progress](progress.md) | Индикатор прогресса (горизонтальный/вертикальный) |
| [Select](select.md) | Выпадающий список для выбора одного значения |
| [Slider](slider.md) | Ползунок для выбора значения из диапазона |
| [Splitter](splitter.md) | Перетаскиваемый разделитель для изменения размера областей |
| [Tooltip](tooltip.md) | Всплывающие подсказки для элементов интерфейса |
| [Tray](tray.md) | Иконка в системном трее с уведомлениями |

## Базовый интерфейс

Все контролы реализуют интерфейс `i_control`:

```cpp
class i_control
{
public:
    virtual void draw(graphic &gr, rect clip) = 0;
    
    virtual void set_position(rect position) = 0;
    virtual rect position() const = 0;
    
    virtual void set_parent(std::shared_ptr<window> window_) = 0;
    virtual std::weak_ptr<window> parent() const = 0;
    virtual void clear_parent() = 0;
    
    virtual void set_topmost(bool yes) = 0;
    virtual bool topmost() const = 0;
    
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool showed() const = 0;
    
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual bool enabled() const = 0;
    
    virtual bool focused() const = 0;
    virtual bool focusing() const = 0;
    
    virtual error get_error() const = 0;
};
```

## Добавление контрола на окно

```cpp
// Создание контрола
auto button = std::make_shared<wui::button>("Click", []() {
    std::cout << "Clicked!" << std::endl;
});

// Добавление на окно с позицией
window->add_control(button, {10, 10, 100, 30}); // left, top, right, bottom
```

## Темизация

Все контролы поддерживают визуальные темы. Пример JSON темы:

```json
{
  "button": {
    "calm": "#06a5df",
    "active": "#1aafe9",
    "disabled": "#a5a5a0"
  },
  "input": {
    "background": "#ffffff",
    "text": "#000000"
  }
}
```

[Подробнее о темах](../base/theme.md)

## См. также

- [Плоская модель владения](../base/ownership.md)
- [События](../base/event.md)
- [Визуальные темы](../base/theme.md)
