#include <memory>

#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(GameWorld, GetEntityById) {
    auto w = GameWorld();
    const auto m = std::make_shared<SingleMachine>(SingleMachine());
    w.addEntity(m);
    auto id = m->getId();
    EXPECT_EQ(w.getEntityById(id)->getId(), id);
}