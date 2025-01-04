//
// Created by Michael Engel on 03.01.25.
//

#ifndef DSL_H
#define DSL_H

#include "../factory.h"

#define LIT(x) #x
#define CONNECTION(a, b) std::make_pair<std::shared_ptr<Fac::GameWorldEntity> const&, int>(a, b)
#define FROM_SLOT0(a) CONNECTION(a, 0)
#define FROM_SLOT1(a) CONNECTION(a, 1)
#define TO_SLOT0(a) CONNECTION(a, 0)
#define TO_SLOT1(a) CONNECTION(a, 1)
#define LINK(from_output, to_input, rpm) linkWithBelt(create(fac, Belt()), from_output, to_input, rpm)
#define LINK_T1(from_output, to_input) linkWithBelt(create(fac, Belt()), from_output, to_input, 60)
#define LINK_T2(from_output, to_input) linkWithBelt(create(fac, Belt()), from_output, to_input, 120)
#define LINK_T3(from_output, to_input) linkWithBelt(create(fac, Belt()), from_output, to_input, 270)
#define LINK_T4(from_output, to_input) linkWithBelt(create(fac, Belt()), from_output, to_input, 480)
#define LINK_T5(from_output, to_input) linkWithBelt(create(fac, Belt()), from_output, to_input, 780)
#define LINK_T6(from_output, to_input) linkWithBelt(create(fac, Belt()), from_output, to_input, 1200)
#define SET_FACTORY(f) auto fac = f;
#define CREATE(id, type) auto const id = create(fac, Fac::type()) ; id->name = "" LIT(id)
#define SMELTER(id, resource) CREATE(id, Machine); id->setRecipe(recipe_##resource)
#define CRAFTER(id, recipe) CREATE(id, Machine); id->setRecipe(recipe_##recipe)
#define ASSEMBLER(id, recipe) auto const id = create(fac, Machine(2, 1)) ; id->name = "" LIT(id) ; id->setRecipe(recipe_##recipe)

// Merger ans Spliiter have no tier, so they operate at each frame
#define SPLITTER(name) CREATE(name, Splitter) ; name->setItemsPerSecond(1000);
#define MERGER(name) CREATE(name, Merger) ; name->setItemsPerSecond(1000);
#define EXTRACTOR_T1(name, node) CREATE(name, Extractor); name->setResourceNode(node) ; name->setDefaultSpeed(60)
#define EXTRACTOR_T2(name, node) CREATE(name, Extractor); name->setResourceNode(node) ; name->setDefaultSpeed(120)
#define EXTRACTOR_T3(name, node) CREATE(name, Extractor); name->setResourceNode(node) ; name->setDefaultSpeed(240)
#define RESOURCE_NODE(name, resource, quality) CREATE(name, ResourceNode) ; name->setResource(Resource::resource); name->setResourceQuality(ResourceQuality::quality)
#define SMALL_STORAGE(name) CREATE(name, Storage) ; name->setMaxItemStacks(12)

typedef std::pair<const std::shared_ptr<Fac::GameWorldEntity>, int> Connection;

namespace DSL {
    template<typename T>
        requires std::derived_from<T, Fac::GameWorldEntity>
    auto create(std::shared_ptr<Fac::Factory> &w, T const &entity) {
        auto e = std::make_shared<T>(entity);
        w->addEntity(e);
        return e;
    }

    inline void connectInput(Connection const &from_input, Connection const &to_output) {
        const auto connector = std::dynamic_pointer_cast<Fac::IInputProvider>(from_input.first);
        connector->connectInput(from_input.second, to_output.first, to_output.second);
    }

   inline  void linkWithBelt(const std::shared_ptr<Fac::Belt> &belt, Connection const &from_output, Connection const &to_input, int const rpm = 60) {
        belt->setItemsPerSecond(rpm / 60.0);
        connectInput(FROM_SLOT0(belt), from_output);
        connectInput(to_input, TO_SLOT0(belt));
    }
}

#endif //DSL_H
