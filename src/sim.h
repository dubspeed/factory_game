#ifndef SIM_H
#define SIM_H

#include <functional>
#include <utility>
#include "core.h"
#include "storage.h"

namespace Fac {


    // System that operates on all SimulatedEntites
    class Simulation {
    public:
        Simulation() = default;

        static void advanceTime(std::vector<std::shared_ptr<SimulatedEntity> > &entities, double dt);
    };


    typedef std::vector<std::variant<
        std::shared_ptr<Stack>,
        std::shared_ptr<SingleMachine>,
        std::shared_ptr<Belt>,
        std::shared_ptr<Merger>,
        std::shared_ptr<Splitter>,
        std::shared_ptr<ResourceNode>,
        std::shared_ptr<ResourceExtractor>,
        std::shared_ptr<Storage>
    > > GameWorldEntities;

    // a global container, that can add and remove game entities
    // and runs the core game loop
    class GameWorld {
        friend void to_json(json &, const GameWorld &);

        friend void from_json(const json &, GameWorld &);

    public:
        GameWorld() = default;

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
            std::shared_ptr<GameWorldEntity> result;
            for (const auto &entity: _entities) {
                std::visit([id, &result](const auto &e) {
                    if (e->getId() == id) {
                        result = std::dynamic_pointer_cast<GameWorldEntity>(e);
                    }
                }, entity);
            }
            return result;
        }

        // templates need to be available at compile time, not just at link time, so
        // these have to be defined here
        template<typename T>
            requires std::derived_from<T, GameWorldEntity>
        void addEntity(std::shared_ptr<T> const &entity) {
            if (auto belt = std::dynamic_pointer_cast<Belt>(entity)) {
                _entities.push_back(belt);
            } else if (auto stack = std::dynamic_pointer_cast<Stack>(entity)) {
                _entities.push_back(stack);
            } else if (auto machine = std::dynamic_pointer_cast<SingleMachine>(entity)) {
                _entities.push_back(machine);
            } else if (auto extr = std::dynamic_pointer_cast<ResourceExtractor>(entity)) {
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
            return true;
        }

        void clearWorld() {
            _entities.clear();
        }

        void update(double dt);

        void advanceBy(double dt, std::function<void()> const &callback);

        void mainLoop();

    private:
        GameWorldEntities _entities;
    };

    void to_json(json &j, const GameWorld &r);

    void from_json(const json &j, GameWorld &r);
}
#endif //SIM_H
