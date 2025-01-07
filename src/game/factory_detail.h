#ifndef FACTORY_DETAIL_H
#define FACTORY_DETAIL_H
#include <__format/format_functions.h>

#include "imgui.h"
#include "../sim.h"
#include "../tools/format.h"

struct FactoryDetailWindowViewModel {
    std::shared_ptr<Fac::Factory> factory;
};

class FactoryDetailWindow final : public Window {
public:
    explicit FactoryDetailWindow(FactoryDetailWindowViewModel model): view_model(std::move(model)) {
    }

    void showFactoryList() const {
        if (ImGui::BeginTable("table1", 7)) {
            for (auto &entity: view_model.factory->getEntities()) {
                ImGui::TableNextRow();
                if (const auto m = std::dynamic_pointer_cast<Fac::Machine>(entity); m) {
                    ImGui::TableNextColumn();
                    ImGui::Checkbox(std::format("##{0}", m->getId()).c_str(), &m->processing);
                    ImGui::TableNextColumn();
                    ImGui::ProgressBar(m->processing_progress / (m->getRecipe().value().processing_time_s * 1000));
                    ImGui::TableNextColumn();
                    auto resourceName = camelCaseToSpaced(
                        Fac::resourceToString(m->getRecipe().value().products.front().resource).data());
                    ImGui::Text(resourceName.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text(std::format("Mach: {0} {1}", m->getId(), m->name).c_str());
                    for (int i = 0; i < m->getInputSlots(); i++) {
                        ImGui::TableNextColumn();
                        ImGui::Text(std::format("I{0}: {1}", i, m->getInputStack(i)->getAmount()).c_str());
                    }
                    for (int i = 0; i < m->getOutputSlots(); i++) {
                        ImGui::TableNextColumn();
                        ImGui::Text(std::format("O{0}: {1}", i, m->getOutputStack(i)->getAmount()).c_str());
                    }
                    ImGui::TableNextColumn();
                    // ImGui::Text(std::format("P: {0}", m->processing).c_str());
                    ImGui::Text(std::format("PPM: {0}", m->getInputRpm()).c_str());

                    // ImGui::Text(std::format("T: {0}", m->getRecipe().value().processing_time_s * 1000 - m->processing_progress).c_str());
                }
            }
            ImGui::EndTable();
        }
    }


    void render(const std::shared_ptr<bool> &is_open) const override {
        if (!ImGui::Begin("Factory Detail", is_open.get(), ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::End();
            return;
        }
        ImGui::Text(STR("Factory Id: {0}", view_model.factory->getId()));

        if (ImGui::BeginTabBar("##tabs")) {
            if (ImGui::BeginTabItem("All")) {
                showFactoryList();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Machines")) {
                ImGui::Text("Machines");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        };
        ImGui::End();
    }

private:
    FactoryDetailWindowViewModel view_model;
};

#endif //FACTORY_DETAIL_H
