# Потоки и таймеры

WUI не гарантирует произвольный конкурентный доступ к контролам. Обновляйте UI
в потоке обработки событий его окна. Внутренние блокировки подписок не делают
`add_control()`, отрисовку и состояние контролов в целом потокобезопасными.

| Платформа | Поток callback `timer` |
| --- | --- |
| Windows | Рабочий поток очереди таймеров Windows |
| Linux | Рабочий поток таймера |
| macOS | Главный поток UI |
| WASM | Главный поток браузера; фоновые вкладки могут замедлять таймеры |

Актуальный API: `timer(callback)`, `start(interval_ms)`, `stop()`.
Конструктор не запускает таймер. Храните его, пока он нужен. Для переносимого UI
отправляйте уведомление окну и меняйте контролы в обработчике события, а не
непосредственно в callback таймера или рабочего потока.

```cpp
#include <wui/system/timer.hpp>

wui::timer timer([weak = std::weak_ptr<wui::window>(window)] {
    if (auto target = weak.lock()) target->emit_event(100, 0);
});
// Handle internal_event_type::user_emitted in the window subscription.
timer.start(100); // milliseconds
// Before captured state or the receiving window is destroyed:
timer.stop();
```

`emit_event(x,y)` доставляет событие `internal_event_type::user_emitted`.
Большие данные передавайте через хранилище приложения с нужной синхронизацией:
два целых аргумента не управляют владением объектами. Остановите таймеры и дождитесь
worker до уничтожения захваченных данных. На Linux нельзя останавливать или
удалять рабочий таймер из его callback: `stop()` делает join его потока.

В WASM пока нет интеграции WUI с pthread/Worker. На macOS вся инициализация UI,
обработка событий и отрисовка выполняются в главном потоке.
