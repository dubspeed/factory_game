#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(SingleMachine, CanProduce) {
    auto m = Machine();
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };
    EXPECT_EQ(m.canStartProduction(), false);
    m.setRecipe(r);
    EXPECT_FALSE(m.canStartProduction());
    m.getInput()->addAmount(5, r.inputs[0].resource);
    EXPECT_TRUE(m.canStartProduction());
    m.getOutputStack(0)->addAmount(MAX_STACK_SIZE, r.products[0].resource);
    EXPECT_FALSE(m.canStartProduction());
    m.getOutputStack(0)->clear();
    m.processing_progress = r.processing_time_s * 1000;
    EXPECT_EQ(m.canStartProduction(), true);
}

TEST(SingleMachine, HasAInputStack) {
    auto m = std::make_shared<Machine>();
    auto s = m->getInputStack(0);
    EXPECT_TRUE(s->isEmpty());
    s->addAmount(MAX_STACK_SIZE, Resource::IronOre);
    EXPECT_EQ(s->getAmount(), MAX_STACK_SIZE);
    EXPECT_EQ(s->getResource(), Resource::IronOre);

    // Link machine to a belt and verify the stack has not changed
    auto b = std::make_shared<Belt>(1);
    m->connectInput(0, b, 0);
    EXPECT_EQ(s->getAmount(), MAX_STACK_SIZE);
    EXPECT_EQ(s->getResource(), Resource::IronOre);
}

TEST(SingleMachine, HasAInputStackIndipendentOfConnectedOutput) {
    auto m = std::make_shared<Machine>();
    auto b = std::make_shared<Belt>(1);
    auto s = m->getInputStack(0);
    EXPECT_TRUE(s->isEmpty());
    s->addAmount(MAX_STACK_SIZE, Resource::IronOre);
    EXPECT_EQ(s->getAmount(), MAX_STACK_SIZE);
    EXPECT_EQ(s->getResource(), Resource::IronOre);
    EXPECT_TRUE(b->getOutputStack(0)->isEmpty());
    b->getOutputStack(0)->addAmount(1, Resource::IronIngot);
    EXPECT_EQ(s->getAmount(), MAX_STACK_SIZE);
    EXPECT_EQ(b->getOutputStack(0)->getAmount(), 1);
}