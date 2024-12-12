#include "sim.h"
#include <chrono>
#include <algorithm>
#include <thread>
#include <iostream>
#include <gtest/internal/gtest-internal.h>

using namespace Fac;

void Simulation::advanceTime(std::vector<std::shared_ptr<SimulatedEntity> > &entities, double dt) {
    for (const auto &entityPtr: entities) {
        entityPtr->update(dt);
    }
}

void GameWorld::update(double const dt) {
    auto simulated_entities = std::vector<std::shared_ptr<SimulatedEntity> >();
    for (auto entity: _entities) {
        std::visit([&simulated_entities](const auto &e) {
            if (std::is_base_of_v<SimulatedEntity, std::decay_t<decltype(*e)>>) {
                simulated_entities.push_back(std::dynamic_pointer_cast<SimulatedEntity>(e));
            }
        }, entity);
    }
    Simulation::advanceTime(simulated_entities, dt);
}

// Updates the Gameworld by 1 ms at a time, until the given time has passed, then calls the callback
void GameWorld::advanceBy(double const dt, std::function<void()> const &callback) {
    for (auto i = 0; i <= dt; i++) {
        update(1);
    }
    callback();
}

void processDeltaTime(double deltaTime) {
    std::cout << "Delta Time: " << deltaTime << " seconds" << std::endl;
}

void GameWorld::mainLoop() {
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint previousTime = Clock::now();

    constexpr int numIterations = 10;

    for (int i = 0; i < numIterations; ++i) {
        // Get the current time at the start of the loop iteration
        TimePoint currentTime = Clock::now();

        // Calculate the duration since the last iteration in seconds
        std::chrono::duration<double> elapsed = currentTime - previousTime;
        const double deltaTime = elapsed.count();

        // Pass deltaTime to the processing function
        processDeltaTime(deltaTime);

        // Update previousTime to the current time for the next iteration
        previousTime = currentTime;

        // Simulate some work by sleeping for 500 milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
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

    // iterate over the entities and reconnect the links
    for (const auto &entity: r._entities) {
        std::visit([&r](const auto &e) {
            if (auto belt = std::dynamic_pointer_cast<Belt>(e)) {
                belt->reconnectLinks([r](const int id) {
                    return r.getEntityById(id);
                });
            }
        }, entity);
    }
}
