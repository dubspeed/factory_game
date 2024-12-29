#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Belt, CanConnectAndMoveItems) {
    auto m1 = std::make_shared<Machine>(Machine());
    m1->setRecipe(recipe_IronIngot);
    auto m2 = std::make_shared<Machine>(Machine());
    m2->setRecipe(recipe_IronPlate);
    auto belt = std::make_shared<Belt>(1);
    belt->connectInput(0, m1, 0);
    m2->connectInput(0, belt, 0);

    m1->getOutputStack(0)->addOne(recipe_IronIngot.products[0].resource);
    belt->update(1);
    // this should move the item from m1 to the belt
    EXPECT_TRUE(m1->getOutputStack(0)->isEmpty());
    EXPECT_EQ(belt->_in_transit_stack.size(), 1);
    EXPECT_EQ(belt->_in_transit_stack[0], Resource::IronIngot);
    EXPECT_TRUE(m2->getInputStack(0)->isEmpty());
    EXPECT_TRUE(belt->getActive());
    EXPECT_FALSE(belt->getJammed());
    // +3 extra frames for the machine and belt to process and activate etc.
    belt->update(999+3);
    EXPECT_EQ(belt->_in_transit_stack.size(), 0);
    EXPECT_EQ(belt->getActive(), false);
    EXPECT_EQ(belt->getJammed(), false);
}

TEST(Belt, TestJammedConnection) {
    auto m1 = std::make_shared<Machine>(Machine());
    m1->setRecipe(recipe_IronIngot);
    auto m2 = std::make_shared<Machine>(Machine());
    m2->setRecipe(recipe_Cable); // Wrong recipe, should jam
    auto belt = std::make_shared<Belt>(1);
    belt->connectInput(0, m1, 0);
    m2->connectInput(0, belt, 0);

    m1->getOutputStack(0)->addOne(recipe_IronIngot.products[0].resource);
    belt->update(1);
    EXPECT_TRUE(m1->getOutputStack(0)->isEmpty());
    EXPECT_EQ(belt->getJammed(), false);


    //belt->update(999);

    // TODO think about JAMMING with new setup
    // The input stack is not empty, because we now link
    // the to the output stack of the belt instead of having
    // their own instance
    //EXPECT_TRUE(m2->getInputStack(0)->isEmpty());
    // EXPECT_EQ(belt->getJammed(), true);
    // EXPECT_EQ(belt->_in_transit_stack.size(), 1);
}