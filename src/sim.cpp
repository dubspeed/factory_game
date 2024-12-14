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


void Fac::to_json(json &j, const GameWorld &r) {
    j["entities"] = json::array();

    // Loop through each entity in the vector
    for (const auto &entity: r._entities) {
        json entity_json;

        std::visit([&entity_json](const auto &e) {
            if (std::is_same_v<std::decay_t<decltype(*e)>, Belt>) {
                entity_json["type"] = "Belt";
                entity_json["data"] = *std::dynamic_pointer_cast<Belt>(e);
            } else if (std::is_same_v<std::decay_t<decltype(*e)>, Stack>) {
                entity_json["type"] = "Stack";
                entity_json["data"] = *std::dynamic_pointer_cast<Stack>(e);
            } else if (std::is_same_v<std::decay_t<decltype(*e)>, SingleMachine>) {
                entity_json["type"] = "SingleMachine";
                entity_json["data"] = *std::dynamic_pointer_cast<SingleMachine>(e);
            } else if (std::is_same_v<std::decay_t<decltype(*e)>, ResourceNode>) {
                entity_json["type"] = "ResourceNode";
                entity_json["data"] = *std::dynamic_pointer_cast<ResourceNode>(e);
            } else if (std::is_same_v<std::decay_t<decltype(*e)>, Merger>) {
                entity_json["type"] = "Merger";
                entity_json["data"] = *std::dynamic_pointer_cast<Merger>(e);
            } else if (std::is_same_v<std::decay_t<decltype(*e)>, Splitter>) {
                entity_json["type"] = "Splitter";
                entity_json["data"] = *std::dynamic_pointer_cast<Splitter>(e);
            } else if (std::is_same_v<std::decay_t<decltype(*e)>, ResourceExtractor>) {
                entity_json["type"] = "ResourceExtractor";
                entity_json["data"] = *std::dynamic_pointer_cast<ResourceExtractor>(e);
            } else if (std::is_same_v<std::decay_t<decltype(*e)>, Storage>) {
                entity_json["type"] = "Storage";
                entity_json["data"] = *std::dynamic_pointer_cast<Storage>(e);
            } else {
                throw std::runtime_error("Unsupported entity type");
            }
        }, entity);

        j["entities"].push_back(entity_json);
    }
}

void Fac::from_json(const json &j, GameWorld &r) {
    r._entities.clear();

    // Loop through each entity in the JSON array
    for (const auto &entity_json: j["entities"]) {
        std::string type = entity_json["type"];

        // Create appropriate variant based on type
        if (type == "Belt") {
            auto b = entity_json["data"].get<Belt>();
            r._entities.push_back(std::make_shared<Belt>(b));
        } else if (type == "Stack") {
            auto s = entity_json["data"].get<Stack>();
            r._entities.push_back(std::make_shared<Stack>(s));
        } else if (type == "SingleMachine") {
            auto m = entity_json["data"].get<SingleMachine>();
            r._entities.push_back(std::make_shared<SingleMachine>(m));
        } else if (type == "ResourceNode") {
            auto m = entity_json["data"].get<ResourceNode>();
            r._entities.push_back(std::make_shared<ResourceNode>(m));
        } else if (type == "ResourceExtractor") {
            auto m = entity_json["data"].get<ResourceExtractor>();
            r._entities.push_back(std::make_shared<ResourceExtractor>(m));
        } else if (type == "Merger") {
            auto m = entity_json["data"].get<Merger>();
            r._entities.push_back(std::make_shared<Merger>(m));
        } else if (type == "Splitter") {
            auto m = entity_json["data"].get<Splitter>();
            r._entities.push_back(std::make_shared<Splitter>(m));
        } else if (type == "Storage") {
            auto m = entity_json["data"].get<Storage>();
            r._entities.push_back(std::make_shared<Storage>(m));
        } else {
            throw std::runtime_error("Unknown entity type in JSON");
        }
    }

    auto fn = [r](const int id) {
        return r.getEntityById(id);
    };

    // iterate over the entities and reconnect the links
    for (const auto &entity: r._entities) {
        std::visit([&r, fn](const auto &e) {
            if (auto belt = std::dynamic_pointer_cast<Belt>(e)) {
                belt->reconnectLinks(fn);
            }
            if (auto machine = std::dynamic_pointer_cast<SingleMachine>(e)) {
                machine->reconnectLinks(fn);
            }
            if (auto merger = std::dynamic_pointer_cast<Merger>(e)) {
                merger->reconnectLinks(fn);
            }
            if (auto splitter = std::dynamic_pointer_cast<Splitter>(e)) {
                splitter->reconnectLinks(fn);
            }
            if (auto storage = std::dynamic_pointer_cast<Storage>(e)) {
                storage->reconnectLinks(fn);
            }
            if (auto extractor = std::dynamic_pointer_cast<ResourceExtractor>(e)) {
                extractor->reconnectLinks(fn);
            }
        }, entity);
    }
}
