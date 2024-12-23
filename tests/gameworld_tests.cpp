#include <memory>

#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(GameWorld, GetEntityById) {
    auto w = Factory();
    const auto m = std::make_shared<Machine>(Machine());
    w.addEntity(m);
    auto id = m->getId();
    EXPECT_EQ(w.getEntityById(id)->getId(), id);
}