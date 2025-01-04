#ifndef GAME_H
#define GAME_H
#include "imgui.h"
#include "../sim.h"
#include <string>
#include <__format/format_functions.h>
#include "../tools/defer.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

struct GameState {
    friend void to_json(json &j, const GameState &r) {
        json factories = json::array();

        for (const auto &factory: r.factories) {
            factories.push_back(*factory);
        }

        j = json{{"gameState", {
            {"credits", r.credits},
            {"resources", r.resources},
            {"factories", factories}
        }}};
    }

    friend void from_json(const json &j, GameState &r) {
        r.credits = j.at("gameState").at("credits").get<float>();
        r.resources = j.at("gameState").at("resources").get<std::map<Fac::Resource, int>>();
        for (const auto &factory: j.at("gameState").at("factories")) {
            r.factories.push_back(std::make_shared<Fac::Factory>(factory.get<Fac::Factory>()));
        }
    }

    // All money the player has
    float credits = 10000.0;

    // All global resources the player has
    std::map<Fac::Resource, int> resources = {};

    // all factories the player owns
    std::vector<std::shared_ptr<Fac::Factory>> factories = {};


};

struct GameWindowViewModel {

    explicit GameWindowViewModel(GameState &state, std::shared_ptr<Fac::Factory> factory): state(state), current_factory(factory) {}

    // The current game state
    GameState& state;

    // The current factory the player is looking at
    std::shared_ptr<Fac::Factory> current_factory;

    void buyFactory() const {
        state.credits -= 1000;
        state.factories.push_back(std::make_shared<Fac::Factory>());
    }

    void sellFactory(int const id) const {
        state.credits += 1000;
        state.factories.erase(std::ranges::remove_if(state.factories, [id](const auto& factory) {
            return factory->getId() == id;
        }).begin(), state.factories.end());
    }
};

#define STR(x, ...) std::format(x, __VA_ARGS__).c_str()

class GameWindow {
public:
    explicit GameWindow(GameWindowViewModel &viewModel): _viewModel(viewModel) {}

    void render() const {
        ImGui::Begin("Game Window");
        ImGui::Text("Credits: %f", _viewModel.state.credits);
        ImGui::Separator();
        ImGui::Text("Factories:");
        if (ImGui::BeginTable("factories_table", 2)) {
            ImGui::TableNextRow();
            for (const auto& factory: _viewModel.state.factories) {
                ImGui::TableNextColumn();
                ImGui::Text("Factory: %d", factory->getId());
                if (ImGui::Button(STR("Sell Factory (+1000)##{0}", factory->getId()))) {
                    auto id = factory->getId();
                    deferCall([this, id]() {
                        _viewModel.sellFactory(id);
                    });
                }
            }
            ImGui::EndTable();
        }
        ImGui::Separator();
        ImGui::Text("Resources:");
        for (const auto& [resource, amount]: _viewModel.state.resources) {
            ImGui::Text("%s: %d", Fac::resourceToString(resource).data(), amount);
        }
        ImGui::Separator();
        if (ImGui::Button("Buy Factory (-1000)")) {
            _viewModel.buyFactory();
        }
        ImGui::End();
    }
private:
    GameWindowViewModel &_viewModel;
};

#endif //GAME_H
