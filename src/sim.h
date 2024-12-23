#ifndef SIM_H
#define SIM_H

#include <functional>
#include <utility>
#include "core.h"
#include "storage.h"

namespace Fac {
    typedef std::vector<std::variant<
        std::shared_ptr<Stack>,
        std::shared_ptr<Machine>,
        std::shared_ptr<Belt>,
        std::shared_ptr<Merger>,
        std::shared_ptr<Splitter>,
        std::shared_ptr<ResourceNode>,
        std::shared_ptr<Extractor>,
        std::shared_ptr<Storage>
    > > GameWorldEntities;


    struct EntityObserver {
        int id;
        std::function<void(std::shared_ptr<GameWorldEntity>)> callback;
    };

    // a global container, that can add and remove game entities
    // and runs the core game loop
    class Factory {
        friend void to_json(json &, const Factory &);

        friend void from_json(const json &, Factory &);

    public:
        Factory() = default;

        // TODO: costly, as it casts each entity to a shared pointer of GameworldEnt
        [[nodiscard]] std::vector<std::shared_ptr<GameWorldEntity> > getEntities() const {
            auto result = std::vector<std::shared_ptr<GameWorldEntity> >();
            result.reserve(_entities.size());

            for (const auto &entity: _entities) {
                std::visit([&result](const auto &e) {
                    using T = std::decay_t<decltype(*e)>;
                    if constexpr (std::is_base_of_v<GameWorldEntity, T>) {
                        result.push_back(std::make_shared<T>(*e));
                    }
                }, entity);
            }
            return result;
        }

        // return the casted entity by id
        [[nodiscard]] std::shared_ptr<GameWorldEntity> getEntityById(int id) const {
            return _entity_map.at(id);
        }

        // templates need to be available at compile time, not just at link time, so
        // these have to be defined here
        template<typename T>
            requires std::derived_from<T, GameWorldEntity>
        void addEntity(std::shared_ptr<T> const &entity) {
            _entity_map[entity->getId()] = entity;
            if (auto belt = std::dynamic_pointer_cast<Belt>(entity)) {
                _entities.push_back(belt);
            } else if (auto stack = std::dynamic_pointer_cast<Stack>(entity)) {
                _entities.push_back(stack);
            } else if (auto machine = std::dynamic_pointer_cast<Machine>(entity)) {
                _entities.push_back(machine);
            } else if (auto extr = std::dynamic_pointer_cast<Extractor>(entity)) {
                _entities.push_back(extr);
            } else if (auto node = std::dynamic_pointer_cast<ResourceNode>(entity)) {
                _entities.push_back(node);
            } else if (auto merger = std::dynamic_pointer_cast<Merger>(entity)) {
                _entities.push_back(merger);
            } else if (auto splitter = std::dynamic_pointer_cast<Splitter>(entity)) {
                _entities.push_back(splitter);
            } else if (auto storage = std::dynamic_pointer_cast<Storage>(entity)) {
                _entities.push_back(storage);
            } else {
                throw std::runtime_error("GameWorld/AddEntity: Can't add unsupported entity type");
            }
        }

        // TODO remove does not seem to work with the variant style
        template<typename T>
            requires std::derived_from<T, GameWorldEntity>
        bool removeEntity(std::shared_ptr<T> const &entity) {
            if (_entities.empty()) {
                return false;
            }
            if (std::find(_entities.begin(), _entities.end(), entity) == _entities.end()) {
                return false;
            }
            std::erase(_entities, entity);
            _entity_map.erase(entity->getId());
            return true;
        }

        void clearWorld() {
            _entities.clear();
            _entity_map.clear();
        }

        void update(double dt) const;

        void advanceBy(double dt, std::function<void()> const &callback) const;

        void processWorldStep() const;

        void registerObserver(int id, std::function<void(std::shared_ptr<GameWorldEntity>)> const &callback) {
            _observers.push_back({id, callback});
        }

    private:
        GameWorldEntities _entities;
        std::vector<EntityObserver> _observers;
        std::map<int, std::shared_ptr<GameWorldEntity> > _entity_map;
    };

    void to_json(json &j, const Factory &r);

    void from_json(const json &j, Factory &r);
}
#endif //SIM_H
