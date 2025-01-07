#ifndef GAME_H
#define GAME_H
#include "imgui.h"
#include "../sim.h"
#include <string>
#include <__format/format_functions.h>

#include "navigation.h"
#include "../tools/defer.h"
#include "../tools/format.h"
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

    // all factories the player owns
    std::vector<std::shared_ptr<Fac::Factory> > factories = {};
};

struct FactoryOverviewWindowViewModel {
    explicit FactoryOverviewWindowViewModel(GameState &state, Navigation &navigation): navigation(navigation), state(state) {
    }

    void buyFactory() const {
        state.credits -= 1000;
        state.factories.push_back(std::make_shared<Fac::Factory>());
    }

    void sellFactory(int const id) const {
        state.credits += 1000;
        state.factories.erase(std::ranges::remove_if(state.factories, [id](const auto &factory) {
            return factory->getId() == id;
        }).begin(), state.factories.end());
    }

    void openFactoryDetailWindow(int const id) const {
        navigation.openWindow(FACTORY_DETAIL, id);
    }

    Navigation &navigation;
    GameState &state;
};


class FactoryOverviewWindow final : public Window {
public:
    explicit FactoryOverviewWindow(const FactoryOverviewWindowViewModel viewModel): _viewModel(std::move(viewModel)) {
    }

    void confirmationPopup(const int id) const {
        if (ImGui::BeginPopupModal(STR("Sell factory?##{0}", id), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Sell factory with everything in it.\nAre you sure?");
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                deferCall([this, id]() {
                    _viewModel.sellFactory(id);
                });
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }
    }

    void render(const std::shared_ptr<bool> &is_open) const override {
        if (!ImGui::Begin("Factory Management Window", is_open.get(), ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::End();
            return;
        }
        ImGui::Text("Credits: %f", _viewModel.state.credits);
        ImGui::Separator();
        if (ImGui::Button("Buy new Factory (-1000)")) {
            _viewModel.buyFactory();
        }
        ImGui::Separator();
        ImGui::Text("Factories:");
        if (_viewModel.state.factories.empty()) {
            ImGui::Text("No factories");
        } else {
            if (ImGui::BeginTable("factories_table", 3)) {
                for (const auto &factory: _viewModel.state.factories) {
                    ImGui::TableNextRow();
                    auto const id = factory->getId();
                    confirmationPopup(id);
                    ImGui::TableNextColumn();
                    ImGui::Text("Factory: %d", id);
                    ImGui::TableNextColumn();
                    if (ImGui::Button(STR("S(+1000)##{0}", id))) {
                        ImGui::OpenPopup(STR("Sell factory?##{0}", id));
                    }
                    ImGui::SetItemTooltip("Sell Factory for 1000 credits");
                    ImGui::TableNextColumn();
                    if (ImGui::Button(STR("Manage##{0}", id))) {
                        _viewModel.openFactoryDetailWindow(id);
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::Separator();
        ImGui::Text("Global Resources:");
        for (const auto &[resource, amount]: _viewModel.state.resources) {
            ImGui::Text("%s: %d", Fac::resourceToString(resource).data(), amount);
        }
        ImGui::Separator();

        ImGui::End();
    }

private:
    FactoryOverviewWindowViewModel _viewModel;
};

#endif //GAME_H
