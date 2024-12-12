#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(OneToOneConnectionTest, CanConnectAndMoveItems) {
    auto m1 = std::make_shared<SingleMachine>(SingleMachine());
    m1->setRecipe(recipe_iron_ingots);
    auto m2 = std::make_shared<SingleMachine>(SingleMachine());
    m2->setRecipe(recipe_iron_plates);
    auto belt = std::make_shared<Belt>(1);
    belt->connectInput(0, m1, 0);
    m2->connectInput(0, belt, 0);

    m1->getOutputStack(0)->addOne(recipe_iron_ingots.products[0].resource);
    belt->update(1);
    // this should move the item from m1 to the belt
    EXPECT_TRUE(m1->getOutputStack(0)->isEmpty());
    EXPECT_EQ(belt->_in_transit_stack.size(), 1);
    EXPECT_EQ(belt->_in_transit_stack[0], Resource::Iron_Ingots);
    EXPECT_TRUE(m2->getInputStack(0)->isEmpty());
    EXPECT_TRUE(belt->getActive());
    EXPECT_FALSE(belt->getJammed());
    belt->update(999);
    // this should move the item from the belt to m2
    EXPECT_EQ(m2->getInputStack(0)->getAmount(), 1);
    EXPECT_EQ(belt->_in_transit_stack.size(), 0);
    EXPECT_EQ(belt->getActive(), false);
    EXPECT_EQ(belt->getJammed(), false);
}

TEST(OneToOneConnectionTest, TestJammedConnection) {
    auto m1 = std::make_shared<SingleMachine>(SingleMachine());
    m1->setRecipe(recipe_iron_ingots);
    auto m2 = std::make_shared<SingleMachine>(SingleMachine());
    m2->setRecipe(recipe_iron_rods); // Wrong recipe, should jam
    auto belt = std::make_shared<Belt>(1);
    belt->connectInput(0, m1, 0);
    m2->connectInput(0, belt, 0);

    // belt.connectToInputStackFrom(TODO, 0);
    // belt.connectToOutputStackFrom(TODO, 0, 0);
    m1->getOutputStack(0)->addOne(recipe_iron_ingots.products[0].resource);
    belt->update(1);
    //EXPECT_TRUE(m1->getOutputStack(0)->isEmpty());
    EXPECT_EQ(belt->getJammed(), false);
    belt->update(999);

    // TODO think about JAMMING with new setup
    // The input stack is not empty, because we now link
    // the to the output stack of the belt instead of having
    // their own instance
    //EXPECT_TRUE(m2->getInputStack(0)->isEmpty());
    EXPECT_EQ(belt->getJammed(), true);
    EXPECT_EQ(belt->_in_transit_stack.size(), 1);
}