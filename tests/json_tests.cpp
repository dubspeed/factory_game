#include <memory>

#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(JSON, AStack) {
    auto s = Stack();
    s.addAmount(10, Resource::Iron_Ore);
    json j = s;
    auto const s2 = j.get<Stack>();
    EXPECT_EQ(s2.getAmount(), 10);
    EXPECT_EQ(s2.getResource(), Resource::Iron_Ore);
}

TEST(JSON, SingleRecipe) {
    auto r = SingleRecipe{Resource::Iron_Ore, 5, Resource::Iron_Ingots, 1, 4};
    json j = r;
    auto const r2 = j.get<SingleRecipe>();
    EXPECT_EQ(r2.amount_in, 5);
    EXPECT_EQ(r2.amount_out, 1);
    EXPECT_EQ(r2.processing_time_s, 4);
    EXPECT_EQ(r2.r_in, Resource::Iron_Ore);
    EXPECT_EQ(r2.r_out, Resource::Iron_Ingots);
    // std::cout << j.dump(4) << std::endl;
}

TEST(JSON, SingleMachine) {
    auto m = SingleMachine();
    auto r = SingleRecipe{Resource::Iron_Ore, 5, Resource::Iron_Ingots, 1, 4};
    m.setRecipe(r);
    m.getInputStack(0)->addAmount(10, Resource::Iron_Ore);
    m.getOutputStack(0)->addAmount(33, Resource::Iron_Ingots);
    json j = m;
    // std::cout << j.dump(4) << std::endl;
    auto const m2 = j.get<SingleMachine>();
    EXPECT_EQ(m2.getOutputRpm(), 15);
    EXPECT_EQ(m2.getInputRpm(), 75);
    EXPECT_EQ(m2.getRecipe().value().amount_in, 5);
    EXPECT_EQ(m2.processing, false);
    EXPECT_EQ(m2.processing_progress, 0.0);
    EXPECT_EQ(m2.getId(), m.getId());

    // Input Stack
    EXPECT_EQ(m2.getInputStack(0)->getAmount(), 10);
    EXPECT_EQ(m2.getInputStack(0)->getResource(), Resource::Iron_Ore);

    // Output Stack
    EXPECT_EQ(m2.getOutputStack(0)->getAmount(), 33);
    EXPECT_EQ(m2.getOutputStack(0)->getResource(), Resource::Iron_Ingots);
}

TEST(JSON, LinkedStacks) {
    auto m = std::make_shared<SingleMachine>();
    auto b = std::make_shared<Belt>(1);
    auto r = SingleRecipe{Resource::Iron_Ore, 5, Resource::Iron_Ingots, 1, 4};
    m->setRecipe(r);
    b->connectInput(0, m, 0);
    m->getOutputStack(0)->addAmount(1, Resource::Iron_Ore);
    json j;
    j["machines"] = *m;
    j["belts"] = *b;
    std::cout << j.dump(4) << std::endl;

    auto m2 = std::make_shared<SingleMachine>(j["machines"].get<SingleMachine>());
    auto b2 = std::make_shared<Belt>(j["belts"].get<Belt>());

    b2->reconnectLinks([m2](int id) {
        return m2;
    });

    EXPECT_EQ(b2->getId(), b->getId());
    EXPECT_EQ(m2->getId(), m->getId());
    EXPECT_EQ(m2->getRecipe().value().amount_in, 5);
    EXPECT_EQ(b2->getInputStack(0).get()->getAmount(), 1);
    EXPECT_EQ(b2->getInputStack(0).get()->getResource(), Resource::Iron_Ore);
}

TEST(JSON, GameWorld) {
    auto w = GameWorld();
    auto m = std::make_shared<SingleMachine>();
    auto b = std::make_shared<Belt>(1);
    auto r = SingleRecipe{Resource::Iron_Ore, 5, Resource::Iron_Ingots, 1, 4};
    m->setRecipe(r);
    b->connectInput(0, m, 0);
    m->getOutputStack(0)->addAmount(1, Resource::Iron_Ingots);
    w.addEntity(m);
    w.addEntity(b);
    json j = w;
    std::cout << j.dump(4) << std::endl;
    w.clearWorld();
    auto w2 = j.get<GameWorld>();
    EXPECT_EQ(w2.getEntities().size(), 2);
    auto m2 = std::dynamic_pointer_cast<SingleMachine>(w2.getEntities()[0]);
    auto b2 = std::dynamic_pointer_cast<Belt>(w2.getEntities()[1]);
    EXPECT_EQ(b2->getId(), b->getId());
    EXPECT_EQ(m2->getId(), m->getId());
    EXPECT_EQ(m2->getRecipe().value().amount_in, 5);
    EXPECT_EQ(b2->getInputStack(0).get()->getAmount(), 1);
    EXPECT_EQ(b2->getInputStack(0).get()->getResource(), Resource::Iron_Ingots);
}