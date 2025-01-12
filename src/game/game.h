#ifndef GAME_H
#define GAME_H

#include "../sim.h"
#include "navigation.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct GameState {
    friend void to_json(json &j, const GameState &r) {
        json factories = json::array();

        for (const auto &factory: r.factories) {
            factories.push_back(*factory);
        }

        j = json{
            {
                "gameState", {
                    {"credits", r.credits},
                    {"resources", r.resources},
                    {"factories", factories}
                }
            }
        };
    }

    friend void from_json(const json &j, GameState &r) {
        r.credits = j.at("gameState").at("credits").get<float>();
        r.resources = j.at("gameState").at("resources").get<std::map<Fac::Resource, int> >();
        for (const auto &factory: j.at("gameState").at("factories")) {
            r.factories.push_back(std::make_shared<Fac::Factory>(factory.get<Fac::Factory>()));
        }
    }

    // All money the player has
    float credits = 10000.0;

    // All global resources the player has
    std::map<Fac::Resource, int> resources = {};

    std::shared_ptr<Fac::Factory> getFactoryById(int const id) {
        return *std::ranges::find_if(factories, [id](const auto &factory) {
            return factory->getId() == id;
        });
    }

    void removeFactoryById(int const id) {
        factories.erase(std::ranges::remove_if(factories, [id](const auto &factory) {
            return factory->getId() == id;
        }).begin(), factories.end());
    }

    void addFactory(std::shared_ptr<Fac::Factory> const &factory) {
        factories.push_back(factory);
    }

    std::vector<std::shared_ptr<Fac::Factory>> getFactories() {
        return factories;
    }



private:
    // all factories the player owns
    std::vector<std::shared_ptr<Fac::Factory> > factories = {};
};

#endif //GAME_H
