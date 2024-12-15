#include <memory>

#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Belt, ConnectTwoBelts) {
    auto w = GameWorld();
    const auto m1 = std::make_shared<SingleMachine>(SingleMachine());
    const auto m2 = std::make_shared<SingleMachine>(SingleMachine());
    const auto belt1 = std::make_shared<Belt>(Belt(1));
    const auto belt2 = std::make_shared<Belt>(Belt(1));

    w.addEntity(m1);
    w.addEntity(m2);
    w.addEntity(belt1);
    w.addEntity(belt2);

    belt1->connectInput(0, m1, 0);
    belt2->connectInput(0, belt1, 0);
    m2->connectInput(0, belt2, 0);

    m1->setRecipe(recipe_IronIngot);
    m2->setRecipe(recipe_IronPlate);

    m1->getInputStack(0)->addAmount(MAX_STACK_SIZE, recipe_IronIngot.inputs[0].resource);
    EXPECT_EQ(m2->getOutputStack(0)->getAmount(), 0);

    w.advanceBy(5 * 60 * 1000, [&]() {
        EXPECT_EQ(m1->getInputStack(0)->getAmount(), 0);
        EXPECT_EQ(m2->getInputStack(0)->getAmount(), 1);
        EXPECT_EQ(belt1->_in_transit_stack.size(), 0);
        EXPECT_EQ(belt2->_in_transit_stack.size(), 0);
        EXPECT_FALSE(belt1->getJammed());
        EXPECT_FALSE(belt2->getJammed());
    });
}

TEST(Belt, ConnectTenBelts) {
    auto w = GameWorld();
    const auto m1 = std::make_shared<SingleMachine>(SingleMachine());
    const auto m2 = std::make_shared<SingleMachine>(SingleMachine());
    w.addEntity(m1);
    w.addEntity(m2);
    m1->setRecipe(recipe_IronIngot);
    m2->setRecipe(recipe_IronPlate);

    const int NUM_BELTS = 10; // TODO make update parallel to allow 1000
    auto belts = std::vector<std::shared_ptr<Belt> >();

    // TODO ideally we would be able to create a thousand belts
    // TODO but this is not possible with the current design
    for (int i = 0; i < NUM_BELTS; i++) {
        auto belt = std::make_shared<Belt>(Belt(1));
        belts.push_back(belt);
        w.addEntity(belt);
    }

    // belts need to be connected in order
    // to make things work, this is a limitation
    belts[0]->connectInput(0, m1, 0);
    for (int i = 1; i <= NUM_BELTS - 2; i++) {
        belts[i]->connectInput(0, belts[i-1], 0);
    }
    belts[NUM_BELTS-1]->connectInput(0, belts[NUM_BELTS-2], 0);
    m2->connectInput(0, belts[NUM_BELTS-1], 0);

    m1->getInputStack(0)->addAmount(MAX_STACK_SIZE, recipe_IronIngot.inputs[0].resource);
    EXPECT_EQ(m2->getOutputStack(0)->getAmount(), 0);

    w.advanceBy(5 * 60 * 1000, [&]() {
        EXPECT_EQ(m1->getOutputStack(0)->getAmount(), 0);
        EXPECT_EQ(m2->getOutputStack(0)->getAmount(), 66);
        for (const auto &belt: belts) {
            EXPECT_EQ(belt->_in_transit_stack.size(), 0);
            EXPECT_FALSE(belt->getJammed());
        }
    });
}

TEST(Belt, HasAOutputStackOfOneItem) {
    auto belt = std::make_shared<Belt>(1);
    belt->getOutputStack(0)->setMaxStackSize(1);
    belt->getOutputStack(0)->addAmount(1, Resource::IronOre);

    EXPECT_EQ(belt->getOutputStack(0)->getAmount(), 1);
    EXPECT_FALSE(belt->getOutputStack(0)->canAdd(1, Resource::IronOre));
    EXPECT_TRUE(belt->getOutputStack(0)->isFull());
}
