#ifndef FACTORY_OVERVIEW_H
#define FACTORY_OVERVIEW_H

#include "game.h"
#include "imgui.h"
#include "navigation.h"
#include "../sim.h"
#include "../tools/format.h"
#include "../tools/gui.h"

struct FactoryOverviewWindowViewModel {
    explicit FactoryOverviewWindowViewModel(GameState &state, Navigation &navigation): navigation(navigation),
        state(state) {
    }

    void buyFactory() const {
        state.credits -= 1000;
        state.addFactory(std::make_shared<Fac::Factory>());
    }

    void sellFactory(int const id) const {
        deferCall([&]() {
            state.credits += 1000;
            state.removeFactoryById(id);
        });
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
                _viewModel.sellFactory(id);
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
        if (_viewModel.state.getFactories().empty()) {
            ImGui::Text("No factories");
        } else {
            if (ImGui::BeginTable("factories_table", 3)) {
                for (const auto &factory: _viewModel.state.getFactories()) {
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
                    if (Gui::StatefulButton(std::format("Manage##{0}", id),
                                            _viewModel.navigation.getWindowOpenState(FACTORY_DETAIL, id))) {
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

#endif //FACTORY_OVERVIEW_H
