#include "sim.h"
#include <chrono>
#include <iostream>

using namespace Fac;

void GameWorld::update(double const dt) const {
    for (const auto& variant : _entities) {
        std::visit([dt](const auto& e) {
            e->update(dt);
        }, variant);
    }
}

// Updates the Gameworld by 1 ms at a time, until the given time has passed, then calls the callback
void GameWorld::advanceBy(double const dt, std::function<void()> const &callback) const {
    for (auto i = 0; i <= dt; i++) {
        update(1);
    }
    callback();
}

void logTimeStep(double deltaTime) {
    std::cout << "DT:" << deltaTime << " ms" << std::endl;
}

void GameWorld::processWorldStep() const {
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    static TimePoint previousTime = Clock::now();

    TimePoint const currentTime = Clock::now();
    std::chrono::duration<double> const elapsed = currentTime - previousTime;
    const double deltaTime = elapsed.count() * 1000;

    update(deltaTime);
    logTimeStep(deltaTime);
    previousTime = currentTime;

    // Notify observers
    for (const auto &entity: _entities) {
        std::visit([this](const auto &e) {
            if (auto gameEntity = std::dynamic_pointer_cast<GameWorldEntity>(e)) {
                auto const id = gameEntity->getId();
                auto observer = std::find_if(_observers.begin(), _observers.end(), [id](const EntityObserver &observer) {
                    return observer.id == id;
                });
                if (observer != _observers.end()) {
                    observer->callback(gameEntity);
                }
            }
        }, entity);
    }
}
