#ifndef SIM_H
#define SIM_H

#include <functional>
#include <memory>
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

        [[nodiscard]] std::vector<std::shared_ptr<GameWorldEntity> > getEntities() const {
            std::vector<std::shared_ptr<GameWorldEntity> > result;
            result.reserve(_entities.size());
            for (const auto &val: _entity_map | std::views::values) {
                result.push_back(val);
            }
            return result;
        }

        // return the cast entity by id
        [[nodiscard]] std::optional<std::shared_ptr<GameWorldEntity>> getEntityById(int const id) const {
            return _entity_map.contains(id) ? std::make_optional(_entity_map.at(id)) : std::nullopt;
        }

        // templates need to be available at compile time, not just at link time, so
        // these have to be defined in the header
        template<typename T>
            requires std::derived_from<T, GameWorldEntity>
        void addEntity(std::shared_ptr<T> const &entity) {
            _entity_map[entity->getId()] = entity;
            _entities.push_back(entity);
        }

        template<typename T>
            requires std::derived_from<T, GameWorldEntity>
        bool removeEntity(std::shared_ptr<T> const &entity) {
            if (_entities.empty()) {
                return false;
            }

            _entities.erase(std::remove_if(_entities.begin(), _entities.end(), [&entity](auto const &e) {
                const std::shared_ptr<GameWorldEntity> basePtr = std::visit(
                    [](const auto &ptr) -> std::shared_ptr<GameWorldEntity> {
                        return ptr; // Implicit upcast to base class pointer
                    }, e);
                return basePtr->getId() == entity->getId();
            }), _entities.end());

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

        void registerObserver(int const id, std::function<void(std::shared_ptr<GameWorldEntity>)> const &callback) {
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
