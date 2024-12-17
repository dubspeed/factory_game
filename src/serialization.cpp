#include "core.h"
#include "sim.h"

#include <functional>
using namespace Fac;

void Fac::to_json(json &j, const SingleMachine &r) {
    auto id = r.getId();
    auto m = json{
        {"id", id}, {"recipe", r.getRecipe()}, {"processing", r.processing}, {"progress", r.processing_progress}
    };
    m["input"] = (InputStackProvider) r;
    m["output"] = (OutputStackProvider) r;
    j["machine"] = m;
}

void Fac::from_json(const json &j, SingleMachine &r) {
    r.setRecipe(j.at("machine").at("recipe").get<Recipe>());
    r.processing = j.at("machine").at("processing").get<bool>();
    r.processing_progress = j.at("machine").at("progress").get<double>();
    r._id = j.at("machine").at("id").get<int>();
    r._input_connections = j.at("machine").at("input").at("_input_connections").get<std::vector<InputConnection> >();
    r._output_stacks = j.at("machine").at("output").at("_output_stacks").get<std::vector<std::shared_ptr<Stack> > >();
}

void Fac::to_json(json &j, const InputConnection &r) {
    if (r.source.lock() == nullptr) {
        j = json{{"stack", *r.cachedStack}};
    } else {
        j = json{{"linkedSourceMachineId", r.sourceId}, {"linkedSourceMachineOutputSlot", r.sourceOutputSlot}};
    }
}

void Fac::from_json(const json &j, InputConnection &r) {
    if (j.contains("stack")) {
        r.cachedStack = std::make_shared<Stack>(j.at("stack").get<Stack>());
    } else {
        r.sourceId = j.at("linkedSourceMachineId").get<int>();
        r.sourceOutputSlot = j.at("linkedSourceMachineOutputSlot").get<int>();
    }
}

void Fac::to_json(json &j, const ResourceNode &r) {
    auto m =
            j["resourceNode"] = json{
                {"id", r.getId()}, {"resource", r.getResource()}, {"quality", r.getQuality()}
            };
}

void Fac::from_json(const json &j, ResourceNode &r) {
    r.setResource(j.at("resourceNode").at("resource").get<Resource>());
    r.setResourceQuality(j.at("resourceNode").at("quality").get<ResourceQuality>());
    r._id = j.at("resourceNode").at("id").get<int>();
}

void Fac::to_json(json &j, const ResourceExtractor &r) {
    auto m = json{
        {"id", r.getId()}, {"extractionProgress", r.extraction_progress}, {"extracting", r.extracting}, {"resNodeId", r._res_node_id}
    };
    m["output"] = (OutputStackProvider) r;
    j["resourceExtractor"] = m;
}

void Fac::from_json(const json &j, ResourceExtractor &r) {
    r.extraction_progress = j.at("resourceExtractor").at("extractionProgress").get<double>();
    r.extracting = j.at("resourceExtractor").at("extracting").get<bool>();
    r._res_node_id = j.at("resourceExtractor").at("resNodeId").get<int>();
    r._id = j.at("resourceExtractor").at("id").get<int>();
    r._output_stacks = j.at("resourceExtractor").at("output").at("_output_stacks").get<std::vector<std::shared_ptr<Stack> > >();
}


// TODO Belt, Splitter and Merger share implementation
void Fac::to_json(json &j, const Belt &r) {
    auto id = r.getId();
    auto m = json{
        {"id", id}, {"itemsPerSecond", r.getItemsPerSecond()}, {"active", r.getActive()}, {"jammed", r.getJammed()}
    };
    m["input"] = (InputStackProvider) r;
    m["output"] = (OutputStackProvider) r;
    m["inTransitStack"] = r._in_transit_stack;
    j["itemMover"] = m;
}

void Fac::from_json(const json &j, Belt &r) {
    r._items_per_s = j.at("itemMover").at("itemsPerSecond").get<int>();
    r._active = j.at("itemMover").at("active").get<bool>();
    r._in_transit_stack = j.at("itemMover").at("inTransitStack").get<std::vector<Resource> >();
    r._jammed = j.at("itemMover").at("jammed").get<bool>();
    r._id = j.at("itemMover").at("id").get<int>();
    r._input_connections = j.at("itemMover").at("input").at("_input_connections").get<std::vector<InputConnection> >();
    r._output_stacks = j.at("itemMover").at("output").at("_output_stacks").get<std::vector<std::shared_ptr<Stack> > >();
}

void Fac::to_json(json &j, const Merger &r) {
    auto id = r.getId();
    auto m = json{
        {"id", id}, {"itemsPerSecond", r.getItemsPerSecond()}, {"active", r.getActive()}, {"jammed", r.getJammed()}
    };
    m["input"] = (InputStackProvider) r;
    m["output"] = (OutputStackProvider) r;
    m["inTransitStack"] = r._in_transit_stack;
    j["itemMover"] = m;
}

void Fac::from_json(const json &j, Merger &r) {
    r._items_per_s = j.at("itemMover").at("itemsPerSecond").get<int>();
    r._active = j.at("itemMover").at("active").get<bool>();
    r._in_transit_stack = j.at("itemMover").at("inTransitStack").get<std::vector<Resource> >();
    r._jammed = j.at("itemMover").at("jammed").get<bool>();
    r._id = j.at("itemMover").at("id").get<int>();
    r._input_connections = j.at("itemMover").at("input").at("_input_connections").get<std::vector<InputConnection> >();
    r._output_stacks = j.at("itemMover").at("output").at("_output_stacks").get<std::vector<std::shared_ptr<Stack> > >();
}

void Fac::to_json(json &j, const Splitter &r) {
    auto id = r.getId();
    auto m = json{
        {"id", id}, {"itemsPerSecond", r.getItemsPerSecond()}, {"active", r.getActive()}, {"jammed", r.getJammed()}
    };
    m["input"] = (InputStackProvider) r;
    m["output"] = (OutputStackProvider) r;
    m["inTransitStack"] = r._in_transit_stack;
    j["itemMover"] = m;
}

void Fac::from_json(const json &j, Splitter &r) {
    r._items_per_s = j.at("itemMover").at("itemsPerSecond").get<int>();
    r._active = j.at("itemMover").at("active").get<bool>();
    r._in_transit_stack = j.at("itemMover").at("inTransitStack").get<std::vector<Resource> >();
    r._jammed = j.at("itemMover").at("jammed").get<bool>();
    r._id = j.at("itemMover").at("id").get<int>();
    r._input_connections = j.at("itemMover").at("input").at("_input_connections").get<std::vector<InputConnection> >();
    r._output_stacks = j.at("itemMover").at("output").at("_output_stacks").get<std::vector<std::shared_ptr<Stack> > >();
}



void Fac::to_json(json &j, const Storage &r) {
    auto id = r.getId();
    auto m = json{
                    {"id", id}, {"max_item_stacks", r._max_item_stacks}
    };
    m["input"] = (InputStackProvider) r;
    m["output"] = (OutputStackProvider) r;
    m["content"] = r._content_stacks;
    j["storage"] = m;
}

void Fac::from_json(const json &j, Storage &r) {
    r._id = j.at("storage").at("id").get<int>();
    r._input_connections = j.at("storage").at("input").at("_input_connections").get<std::vector<InputConnection>>();
    r._output_stacks = j.at("storage").at("output").at("_output_stacks").get<std::vector<std::shared_ptr<Stack>>>();
    r._max_item_stacks = j.at("storage").at("max_item_stacks").get<int>();
    r._content_stacks = j.at("storage").at("content").get<std::vector<Stack>>();
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
    r._entity_map.clear();

    // Loop through each entity in the JSON array
    for (const auto &entity_json: j["entities"]) {
        std::string type = entity_json["type"];

        // Create appropriate variant based on type
        if (type == "Belt") {
            auto b = entity_json["data"].get<Belt>();
            auto belt = std::make_shared<Belt>(b);
            r._entities.push_back(belt);
            r._entity_map[belt->getId()] = belt;
        } else if (type == "Stack") {
            auto s = entity_json["data"].get<Stack>();
            auto stack = std::make_shared<Stack>(s);
            r._entities.push_back(stack);
            r._entity_map[stack->getId()] = stack;
        } else if (type == "SingleMachine") {
            auto m = entity_json["data"].get<SingleMachine>();
            auto machine = std::make_shared<SingleMachine>(m);
            r._entities.push_back(machine);
            r._entity_map[machine->getId()] = machine;
        } else if (type == "ResourceNode") {
            auto m = entity_json["data"].get<ResourceNode>();
            auto node = std::make_shared<ResourceNode>(m);
            r._entities.push_back(node);
            r._entity_map[node->getId()] = node;
        } else if (type == "ResourceExtractor") {
            auto m = entity_json["data"].get<ResourceExtractor>();
            auto extractor = std::make_shared<ResourceExtractor>(m);
            r._entities.push_back(extractor);
            r._entity_map[extractor->getId()] = extractor;
        } else if (type == "Merger") {
            auto m = entity_json["data"].get<Merger>();
            auto merger = std::make_shared<Merger>(m);
            r._entities.push_back(merger);
            r._entity_map[merger->getId()] = merger;
        } else if (type == "Splitter") {
            auto m = entity_json["data"].get<Splitter>();
            auto splitter = std::make_shared<Splitter>(m);
            r._entities.push_back(splitter);
            r._entity_map[splitter->getId()] = splitter;
        } else if (type == "Storage") {
            auto m = entity_json["data"].get<Storage>();
            auto storage = std::make_shared<Storage>(m);
            r._entities.push_back(storage);
            r._entity_map[storage->getId()] = storage;
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
            if (auto link = std::dynamic_pointer_cast<IInputLink>(e)) {
                link->reconnectLinks(fn);
            }
        }, entity);
    }
}
