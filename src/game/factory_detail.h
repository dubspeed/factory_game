#ifndef FACTORY_DETAIL_H
#define FACTORY_DETAIL_H
#include <__format/format_functions.h>

#include "imgui.h"
#include "../sim.h"
#include "../tools/format.h"

using namespace ImGui;
using namespace Fac;
using namespace Gui;

struct FactoryDetailWindowViewModel {
    std::shared_ptr<Factory> factory;

    void sellExtractor(const std::shared_ptr<Extractor> &extractor) const {
        deferCall([=]() {
            factory->removeEntity(extractor);
        });
    }

    void connectExtractorToResourceNode(const std::shared_ptr<Extractor> &extractor,
                                        int const resourceNodeId) const {
        deferCall([=] {
            extractor->setResourceNode(
                std::dynamic_pointer_cast<ResourceNode>(factory->getEntityById(resourceNodeId).value()));
        });
    }

    auto getUnconnectedExtractors() const {
        const auto ent = factory->getEntitiesByType<Extractor>();
        std::vector<std::shared_ptr<Extractor> > result;
        for (const auto &e: ent) {
            if (!e->hasResourceNode()) {
                result.push_back(e);
            }
        }
        return result;
    }

    std::shared_ptr<Extractor> resourceNodeHasExtractor(const std::shared_ptr<ResourceNode> &node) const {
        for (const auto &e: factory->getEntitiesByType<Extractor>()) {
            if (e->getResourceNode()->getId() == node->getId()) {
                return e;
            }
        }
        return nullptr;
    }

    void disconnectExtractorFromResourceNode(const std::shared_ptr<Extractor> & extractor) const {
        extractor->clearResourceNode();
    }
};

class FactoryDetailWindow final : public Window {
public:
    explicit FactoryDetailWindow(FactoryDetailWindowViewModel model): view_model(std::move(model)) {
    }

    void showFactoryList() const {
        if (BeginTable("table1", 7)) {
            for (auto &entity: view_model.factory->getEntities()) {
                TableNextRow();
                if (const auto m = std::dynamic_pointer_cast<Machine>(entity); m) {
                    TableNextColumn();
                    Checkbox(std::format("##{0}", m->getId()).c_str(), &m->processing);
                    TableNextColumn();
                    ProgressBar(m->processing_progress / (m->getRecipe().value().processing_time_s * 1000));
                    TableNextColumn();
                    auto resourceName = camelCaseToSpaced(
                        resourceToString(m->getRecipe().value().products.front().resource).data());
                    Text(resourceName.c_str());
                    TableNextColumn();
                    Text(std::format("Mach: {0} {1}", m->getId(), m->name).c_str());
                    for (int i = 0; i < m->getInputSlots(); i++) {
                        TableNextColumn();
                        Text(std::format("I{0}: {1}", i, m->getInputStack(i)->getAmount()).c_str());
                    }
                    for (int i = 0; i < m->getOutputSlots(); i++) {
                        TableNextColumn();
                        Text(std::format("O{0}: {1}", i, m->getOutputStack(i)->getAmount()).c_str());
                    }
                    TableNextColumn();
                    // Text(std::format("P: {0}", m->processing).c_str());
                    Text(std::format("PPM: {0}", m->getInputRpm()).c_str());

                    // Text(std::format("T: {0}", m->getRecipe().value().processing_time_s * 1000 - m->processing_progress).c_str());
                }
            }
            EndTable();
        }
    }

    void listResourceNodes() const {
        if (!BeginTable("table_resource_nodes", 7)) return;

        for (const auto &m: view_model.factory->getEntitiesByType<ResourceNode>()) {
            auto id = m->getId();

            connectionPopup(id);
            TableNextRow();
            TableNextColumn();
            auto resourceName = camelCaseToSpaced(resourceToString(m->getResource()).data());
            Text(resourceName.c_str());
            TableNextColumn();
            auto quality = resourceQualityToString(m->getQuality());
            Text(quality.data());
            TableNextColumn();
            if (view_model.resourceNodeHasExtractor(m) != nullptr) {
                Text("Connected");
            } else {
                Text("Unconnected");
            }
            TableNextColumn();
            if (auto e = view_model.resourceNodeHasExtractor(m); e != nullptr) {
                if (Button(STR("Disconnect##{0}", id))) {
                    view_model.disconnectExtractorFromResourceNode(e);
                }
            } else {
                if (Button(STR("Connect##{0}", id))) {
                    OpenPopup(STR("Connect#{0}", id));
                }
            }

        }
        EndTable();
    }

    void connectionPopup(const int id) const {
        if (!BeginPopupModal(STR("Connect#{0}", id), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

        Text("Connect to Resource Node");
        Separator();
        for (const auto &extractor: view_model.getUnconnectedExtractors()) {

            if (Selectable(STR("Extractor {0}", extractor->getId()))) {
                view_model.connectExtractorToResourceNode(extractor, id);
            }
        }
        if (Button("OK", ImVec2(120, 0))) {
            CloseCurrentPopup();
            //view_model.connectToResourceNode(id);
        }
        SetItemDefaultFocus();
        SameLine();
        if (Button("Cancel", ImVec2(120, 0))) { CloseCurrentPopup(); }
        EndPopup();
    }

    void listExtractors() const {
        if (!BeginTable("table_extractors", 7))
            return;

        for (auto &m: view_model.factory->getEntitiesByType<Extractor>()) {
            auto id = m->getId();
            TableNextRow();

            TableNextColumn();
            Text(STR("#{0}", id));
            TableNextColumn();
            BeginDisabled();
            Checkbox(STR("Active##{0}", id), &m->extracting);
            EndDisabled();
            TableNextColumn();
            ProgressBar(m->extraction_progress / 100);
            TableNextColumn();
            auto resource_node = camelCaseToSpaced(
                resourceToString(m->getResourceNode()->getResource()).data());
            Text(resource_node.c_str());
            TableNextColumn();
            Text(STR("{0}", m->getOutputRpm()));
            TableNextColumn();
            auto output_amount = m->getOutputStack(0)->getAmount();
            Text(STR("{0}", output_amount));
            TableNextColumn();
            if (Button(STR("Sell##{0}", id))) {
                view_model.sellExtractor(m);
            }
        }
        EndTable();
    }


    void render(const std::shared_ptr<bool> &is_open) const override {
        auto id = view_model.factory->getId();
        int selected_fish = -1;
        const char *names[] = {"IronOre", "CopperOre", "Coal", "Stone", "CrudeOil", "Water", "Uranium"};

        if (!Begin(STR("Factory Detail##{0}", id), is_open.get())) {
            End();
            return;
        }
        Text(STR("Factory Id: {0}", id));

        if (BeginTabBar("##tabs")) {
            if (BeginTabItem("Resources")) {
                listResourceNodes();
                EndTabItem();
            }

            if (BeginTabItem("Extractors")) {
                listExtractors();
                EndTabItem();
                Separator();
                if (Button("Select.."))
                    OpenPopup("select_resource_node");
                SameLine();
                TextUnformatted(selected_fish == -1 ? "<None>" : names[selected_fish]);
                if (BeginPopup("select_resource_node")) {
                    SeparatorText("Resource Nodes");
                    for (int i = 0; i < IM_ARRAYSIZE(names); i++)
                        if (Selectable(names[i]))
                            selected_fish = i;
                    EndPopup();
                }
            }

            // if (BeginTabItem("All")) {
            //     showFactoryList();
            //     EndTabItem();
            // }

            // if (BeginTabItem("Machines")) {
            //     Text("Machines");
            //     EndTabItem();
            // }

            EndTabBar();
        };
        End();
    }

private:
    FactoryDetailWindowViewModel view_model;
};

#endif //FACTORY_DETAIL_H
