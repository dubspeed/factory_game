#ifndef NAVIGATION_H
#define NAVIGATION_H
#include <functional>
#include <map>


enum WindowType {
    OVERVIEW,
    FACTORY_DETAIL,
    DEMO,
};

class Window {
public:
    virtual ~Window() = default;

    virtual void render(const std::shared_ptr<bool> &p_open) const = 0;
};

using ConstructorFunction = std::variant<
    std::function<std::unique_ptr<Window>()>,
    std::function<std::unique_ptr<Window>(int)>
>;

// using RenderFunction = const std::function<void(const std::unique_ptr<bool> &p_open)>;

template<typename T>
inline constexpr bool always_false = false;

struct WindowInfo {
    ConstructorFunction constructor_function;
    std::shared_ptr<bool> is_open = std::make_shared<bool>(true);
    std::unique_ptr<Window> instance = nullptr;

    void createInstance() {
        if (instance != nullptr) return;

        instance = std::visit([](const auto &f)-> std::unique_ptr<Window> {
            using F = std::decay_t<decltype(f)>;
            if constexpr (std::is_same_v<F, std::function<std::unique_ptr<Window>()> >) {
                return f();
            } else if constexpr (std::is_same_v<F, std::function<std::unique_ptr<Window>(int)> >) {
                return f(0);
            } else {
                static_assert(always_false<F>, "Unsupported function type");
            }
        }, constructor_function);
    }

    void createInstance(const int id) {
        if (instance != nullptr) return;

        instance = std::visit([&](const auto &f)-> std::unique_ptr<Window> {
            using F = std::decay_t<decltype(f)>;
            if constexpr (std::is_same_v<F, std::function<std::unique_ptr<Window>()> >) {
                return f();
            } else if constexpr (std::is_same_v<F, std::function<std::unique_ptr<Window>(int)> >) {
                return f(id);
            } else {
                static_assert(always_false<F>, "Unsupported function type");
            }
        }, constructor_function);
    }

    void render() const {
        if (instance == nullptr) return;
        if (*is_open == true) instance->render(is_open);
    }
};


class FactoryOverviewWindow;

struct Navigation {
    std::map<WindowType, WindowInfo> windows;

    void registerWindow(
        WindowType const type,
        const std::function<std::unique_ptr<Window>()> &constructorFunction) {
        windows[type] = {constructorFunction};
    }

    void registerWindow(
        WindowType const type,
        const std::function<std::unique_ptr<Window>(int)> &constructorFunction) {
        windows[type] = {constructorFunction};
    }

    void openWindow(WindowType const type) {
        if (windows.contains(type)) {
            windows[type].is_open = std::make_unique<bool>(true);
            windows[type].createInstance();
        }
    }

    void openWindow(WindowType const type, int const id) {
        if (windows.contains(type)) {
            windows[type].is_open = std::make_shared<bool>(true);
            windows[type].createInstance(id);
        }
    }

    // Render all windows
    void render() {
        for (auto &val: windows | std::views::values) {
            if (val.instance != nullptr && *val.is_open == true)
                val.render();
        }
    }

    std::shared_ptr<bool> getWindowOpenState(WindowType const type) {
        if (windows.contains(type)) {
            return windows[type].is_open;
        }
        return nullptr;
    }
};
#endif //NAVIGATION_H
