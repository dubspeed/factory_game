#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(TwoMachineProductionChain, WithSimpleBelt) {
    auto w = Factory();
    const auto m1 = std::make_shared<Machine>(Machine());
    const auto m2 = std::make_shared<Machine>(Machine());
    const auto belt = std::make_shared<Belt>(Belt(1));

    w.addEntity(m1);
    w.addEntity(m2);
    w.addEntity(belt);

    m1->setRecipe(recipe_IronIngot);
    m2->setRecipe(recipe_IronPlate);
    belt->connectInput(0, m1, 0);
    m2->connectInput(0, belt, 0);

    m1->getInputStack(0)->addAmount(MAX_STACK_SIZE, Resource::IronOre);
    EXPECT_TRUE(m2->getOutputStack(0)->isEmpty());

    // first ingot is produced
    w.advanceBy(2000 + 100, [&]() {
        EXPECT_TRUE(m1->getOutputStack(0)->isEmpty());
        EXPECT_TRUE(m2->getInputStack(0)->isEmpty());
        EXPECT_EQ(belt->_in_transit_stack.size(), 1);
    });
    // first ingot shipped
    w.advanceBy(1000, [&]() {
        EXPECT_TRUE(m1->getOutputStack(0)->isEmpty());
        EXPECT_EQ(m2->getInputStack(0)->getAmount(), 1);
        EXPECT_EQ(belt->_in_transit_stack.size(), 0);
    });
    // somewhere in the middle of production
    w.advanceBy(105333, [&]() {
        EXPECT_EQ(m1->getInputStack(0)->getAmount(), 45);
        EXPECT_EQ(m2->getOutputStack(0)->getAmount(), 32);
        EXPECT_EQ(belt->_in_transit_stack.size(), 1);
    });
    // first machine is finished last ingot shipping
    w.advanceBy(90000, [&]() {
       EXPECT_TRUE(m1->getInputStack(0)->isEmpty());
        EXPECT_TRUE(m1->getOutputStack(0)->isEmpty());
       EXPECT_EQ(m2->getInputStack(0)->getAmount(), 2);
       EXPECT_EQ(m2->getOutputStack(0)->getAmount(), 62);
       EXPECT_EQ(belt->_in_transit_stack.size(), 1);
   });
    // production finished with 66 plates
    w.advanceBy(7000, [&]() {
        EXPECT_EQ(m1->getInputStack(0)->getAmount(), 0);
        EXPECT_EQ(m2->getInputStack(0)->getAmount(), 1);
        EXPECT_EQ(m2->getOutputStack(0)->getAmount(), 66);
        EXPECT_EQ(belt->_in_transit_stack.size(), 0);
    });
}

TEST(DepositToMachine, WithSimpleBelt) {
    auto w = Factory();
    const auto n1 = std::make_shared<ResourceNode>(ResourceNode());
    const auto m1 = std::make_shared<Machine>(Machine());
    const auto e1 = std::make_shared<Extractor>(Extractor());

    n1->setResource(Resource::IronOre, ResourceQuality::Normal);
    // belt between extractor and machine
    const auto belt1 = std::make_shared<Belt>(Belt(1));

    // Place extractor on resource node
    e1->setResourceNode(n1);
    belt1->connectInput(0, e1, 0);
    m1->connectInput(0, belt1, 0);

    // add node and extractor to the world
    w.addEntity(n1);
    w.addEntity(e1);

    // add belt and machine to the world
    w.addEntity(m1);
    w.addEntity(belt1);

    // set recipe for the machine
    m1->setRecipe(recipe_IronIngot);

    // after around 2 minutes, we should have about 59 ingots
    w.advanceBy(2 * 60 * 1000, [&]() {
        EXPECT_TRUE(e1->getOutputStack(0)->isEmpty());
        EXPECT_TRUE(m1->processing);
        EXPECT_EQ(m1->getInputStack(0)->getAmount(), 59);
        EXPECT_EQ(m1->getOutputStack(0)->getAmount(), 58);
        EXPECT_EQ(belt1->_in_transit_stack.size(), 1);
        EXPECT_FALSE(belt1->getJammed());
    });

    // produce for 3 more minutes and we are full and jammed
    w.advanceBy(5 * 60 * 1000, [&]() {
        EXPECT_EQ(e1->getOutputStack(0)->getAmount(), 100);
        EXPECT_FALSE(m1->processing);
        EXPECT_EQ(m1->getInputStack(0)->getAmount(), 100);
        EXPECT_EQ(m1->getOutputStack(0)->getAmount(), 100);
        EXPECT_EQ(belt1->_in_transit_stack.size(), 1);
        EXPECT_TRUE(belt1->getJammed());
    });


}