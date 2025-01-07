#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Fac {
    class GameWorldEntity;
    class InputStackProvider;
    class OutputStackProvider;
    struct InputConnection;
    struct Stack;
    class Extractor;
    class ResourceNode;
    class Belt;
    class Splitter;
    class Merger;
    class Machine;
}

// For custom types like std::optional
namespace nlohmann {
    template<typename T>
    struct adl_serializer<std::optional<T> > {
        static void to_json(json &j, const std::optional<T> &opt) {
            if (opt.has_value()) {
                j = *opt;
            } else {
                j = nullptr;
            }
        }

        static void from_json(const json &j, std::optional<T> &opt) {
            if (j.is_null()) {
                opt = std::nullopt;
            } else {
                opt = j.get<T>();
            }
        }
    };

    template<typename T>
    requires std::derived_from<T, Fac::Stack>
    struct adl_serializer<std::vector<std::shared_ptr<T> > > {
        static void to_json(json &j, const std::vector<std::shared_ptr<T> > &stacks) {
            j = json::array();
            for (const auto &stack: stacks) {
                json sj = *stack;
                j.push_back(sj);
            }
        }

        static void from_json(const json &j, std::vector<std::shared_ptr<T> > &stacks) {
            stacks.clear();
            for (const auto &stack: j) {
                stacks.push_back(std::make_shared<T>(stack.get<T>()));
            }
        }
    };
}

namespace Fac {
    void to_json(json &j, const InputConnection &r);

    void from_json(const json &j, InputConnection &r);

    void to_json(json &j, const ResourceNode &r);

    void from_json(const json &j, ResourceNode &r);

    void to_json(json &j, const Extractor &r);

    void from_json(const json &j, Extractor &r);

    void to_json(json &j, const Machine &r);

    void from_json(const json &j, Machine &r);

    void to_json(json &j, const Belt &r);

    void from_json(const json &j, Belt &r);

    void to_json(json &j, const Splitter &r);

    void from_json(const json &j, Splitter &r);

    void to_json(json &j, const Merger &r);

    void from_json(const json &j, Merger &r);
}

#endif //SERIALIZATION_H
