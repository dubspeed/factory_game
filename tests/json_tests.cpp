#include <memory>

#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(JSON, AStack) {
    auto s = Stack();
    s.addAmount(10, Resource::IronOre);
    json j = s;
    auto const s2 = j.get<Stack>();
    EXPECT_EQ(s2.getAmount(), 10);
    EXPECT_EQ(s2.getResource(), Resource::IronOre);
}

TEST(JSON, RecipeTest) {
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };
    json j = r;
    auto const r2 = j.get<Recipe>();
    EXPECT_EQ(r2.inputs[0].amount, 5);
    EXPECT_EQ(r2.products[0].amount, 1);
    EXPECT_EQ(r2.processing_time_s, 4);
    EXPECT_EQ(r2.inputs[0].resource, Resource::IronOre);
    EXPECT_EQ(r2.products[0].resource, Resource::IronIngot);
    std::cout << j.dump(4) << std::endl;
}

TEST(JSON, Machine) {
    auto m = Machine();
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };
    m.setRecipe(r);
    m.getInputStack(0)->addAmount(10, Resource::IronOre);
    m.getOutputStack(0)->addAmount(33, Resource::IronIngot);
    json j = m;
    std::cout << j.dump(4) << std::endl;
    auto const m2 = j.get<Machine>();
    EXPECT_EQ(m2.getOutputRpm(), 15);
    EXPECT_EQ(m2.getInputRpm(), 75);
    EXPECT_EQ(m2.getRecipe().value().inputs[0].amount, 5);
    EXPECT_EQ(m2.processing, false);
    EXPECT_EQ(m2.processing_progress, 0.0);
    EXPECT_EQ(m2.getId(), m.getId());

    // Input Stack
    EXPECT_EQ(m2.getInputStack(0)->getAmount(), 10);
    EXPECT_EQ(m2.getInputStack(0)->getResource(), Resource::IronOre);

    // Output Stack
    EXPECT_EQ(m2.getOutputStack(0)->getAmount(), 33);
    EXPECT_EQ(m2.getOutputStack(0)->getResource(), Resource::IronIngot);
}

TEST(JSON, LinkedStacks) {
    auto m = std::make_shared<Machine>();
    auto b = std::make_shared<Belt>(1);
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };
    m->setRecipe(r);
    b->connectInput(0, m, 0);
    m->getOutputStack(0)->addAmount(1, Resource::IronOre);
    json j;
    j["machines"] = *m;
    j["belts"] = *b;
    std::cout << j.dump(4) << std::endl;

    auto m2 = std::make_shared<Machine>(j["machines"].get<Machine>());
    auto b2 = std::make_shared<Belt>(j["belts"].get<Belt>());

    b2->reconnectLinks([m2](int id) {
        return m2;
    });

    EXPECT_EQ(b2->getId(), b->getId());
    EXPECT_EQ(m2->getId(), m->getId());
    EXPECT_EQ(m2->getRecipe().value().inputs[0].amount, 5);
    EXPECT_EQ(b2->getInputStack(0).get()->getAmount(), 1);
    EXPECT_EQ(b2->getInputStack(0).get()->getResource(), Resource::IronOre);
}

TEST(JSON, GameWorld) {
    auto w = Factory();
    auto m = std::make_shared<Machine>();
    auto b = std::make_shared<Belt>(1);
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };
    m->setRecipe(r);
    b->connectInput(0, m, 0);
    m->getOutputStack(0)->addAmount(1, Resource::IronIngot);
    w.addEntity(m);
    w.addEntity(b);
    json j = w;
    // std::cout << j.dump(4) << std::endl;
    w.clearWorld();
    auto w2 = j.get<Factory>();
    EXPECT_EQ(w2.getEntities().size(), 2);
    auto m2 = std::dynamic_pointer_cast<Machine>(w2.getEntities()[0]);
    auto b2 = std::dynamic_pointer_cast<Belt>(w2.getEntities()[1]);
    EXPECT_EQ(b2->getId(), b->getId());
    EXPECT_EQ(m2->getId(), m->getId());
    EXPECT_EQ(m2->getRecipe().value().inputs[0].amount, 5);
    EXPECT_EQ(b2->getInputStack(0).get()->getAmount(), 1);
    EXPECT_EQ(b2->getInputStack(0).get()->getResource(), Resource::IronIngot);
}

TEST(JSON, ResourceExtractor) {
    // TBD
}