#ifndef NAVIGATION_H
#define NAVIGATION_H
#include <functional>
#include <map>

enum WindowType {
    OVERVIEW,
    FACTORY_DETAIL,
    DEMO,
};

inline static int constexpr DEFAULT_ID = 10000;

class Window {
public:
    virtual ~Window() = default;

    virtual void render(const std::shared_ptr<bool> &p_open) const = 0;
};

using ConstructorFunction = std::variant<
    std::function<std::unique_ptr<Window>()>,
    std::function<std::unique_ptr<Window>(int)>
>;

template<typename T>
inline constexpr bool always_false = false;

struct WindowInfo {
    int window_id = 0;
    ConstructorFunction constructor_function;
    std::shared_ptr<bool> is_open = std::make_shared<bool>(true);
    std::unique_ptr<Window> instance = nullptr;
    
    void createInstance(const int id=0) {
        if (instance != nullptr) return;

        instance = std::visit([&]<typename T>(const T &f)-> std::unique_ptr<Window> {
            using F = std::decay_t<T>;
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
    std::map<WindowType, ConstructorFunction> constructors;
    std::map<WindowType, std::map<int, WindowInfo>> windows;

    void registerWindow(
        WindowType const type,
        const std::function<std::unique_ptr<Window>()> &constructorFunction) {
        constructors[type] = constructorFunction;
    }

    void registerWindow(
        WindowType const type,
        const std::function<std::unique_ptr<Window>(int)> &constructorFunction) {
        constructors[type] = constructorFunction;
    }

    void openWindow(WindowType const type, int const id = DEFAULT_ID) {
        if (constructors.contains(type)) {
            if (windows[type].contains(id)) {
                *windows[type][id].is_open = true;
            } else {
                windows[type][id] = {id, constructors[type]};
                windows[type][id].createInstance(id);
            }
        }
    }

    // Render all windows
    void render() {
        for (auto &idmap: windows | std::views::values) {
            for (auto &val: idmap | std::views::values) {
                if (val.instance != nullptr && *val.is_open == true)
                    val.render();
            }
        }
    }

    std::shared_ptr<bool> getWindowOpenState(WindowType const type) {
        return getWindowOpenState(type, DEFAULT_ID);
    }

    std::shared_ptr<bool> getWindowOpenState(WindowType const type, const int id) {
        if (windows.contains(type)) {
            if (windows[type].contains(id)) return windows[type][id].is_open;
        }
        return nullptr;
    }

};
#endif //NAVIGATION_H
