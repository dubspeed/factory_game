#include <memory>

#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Factory, GetEntityById) {
    auto w = Factory();
    const auto m = std::make_shared<Machine>(Machine());
    w.addEntity(m);
    auto id = m->getId();
    EXPECT_EQ(w.getEntityById(id).value()->getId(), id);
}

TEST(Factory, GetEntitiesByType) {
    auto f = Factory();
    const auto m1 = std::make_shared<Machine>(Machine());
    const auto m2 = std::make_shared<Machine>(Machine());
    const auto b = std::make_shared<Belt>(Belt(1));
    f.addEntity(m1);
    f.addEntity(m2);
    f.addEntity(b);
    auto machines = f.getEntitiesByType(typeid(Machine));
    EXPECT_EQ(machines.size(), 2);
    EXPECT_EQ(machines[0]->getId(), m1->getId());
    EXPECT_EQ(machines[1]->getId(), m2->getId());
    auto belts = f.getEntitiesByType(typeid(Belt));
    EXPECT_EQ(belts.size(), 1);
    EXPECT_EQ(belts[0]->getId(), b->getId());

}