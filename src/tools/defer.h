#ifndef DEFER_H
#define DEFER_H

#include <functional>

inline void deferCall(const std::function<void()> &f) {
    static std::vector<std::function<void()>> deferred;

    if (f == nullptr) {
        for (auto fun : deferred) {
            fun();
        }
        deferred.clear();
    } else {
        deferred.push_back(f);
    }
}

#endif //DEFER_H
