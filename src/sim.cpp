#include "sim.h"
#include <chrono>
#include <iostream>

using namespace Fac;

void Simulation::advanceTime(std::vector<std::shared_ptr<SimulatedEntity> > &entities, double dt) {
    for (const auto &entityPtr: entities) {
        entityPtr->update(dt);
    }
}

void GameWorld::update(double const dt) const {
    auto simulated_entities = std::vector<std::shared_ptr<SimulatedEntity> >();
    for (auto entity: _entities) {
        std::visit([&simulated_entities](const auto &e) {
            if (std::is_base_of_v<SimulatedEntity, std::decay_t<decltype(*e)> >) {
                simulated_entities.push_back(std::dynamic_pointer_cast<SimulatedEntity>(e));
            }
        }, entity);
    }
    Simulation::advanceTime(simulated_entities, dt);
}

// Updates the Gameworld by 1 ms at a time, until the given time has passed, then calls the callback
void GameWorld::advanceBy(double const dt, std::function<void()> const &callback) const {
    for (auto i = 0; i <= dt; i++) {
        update(1);
    }
    callback();
}

void logTimeStep(double deltaTime) {
    std::cout << "Delta Time: " << deltaTime << " seconds" << std::endl;
}

void GameWorld::processWorldStep() const {
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    static TimePoint previousTime = Clock::now();

    TimePoint const currentTime = Clock::now();
    std::chrono::duration<double> const elapsed = currentTime - previousTime;
    const double deltaTime = elapsed.count();

    update(deltaTime * 1000);
    //logTimeStep(deltaTime);
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
