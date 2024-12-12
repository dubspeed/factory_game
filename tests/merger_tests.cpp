#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Merger, CanMergeTwoBelts) {
    auto w = GameWorld();
    auto mg = std::make_shared<Merger>(Merger());
    auto belt1 = std::make_shared<Belt>(Belt(1));
    auto belt2 = std::make_shared<Belt>(Belt(1));
    auto belt3 = std::make_shared<Belt>(Belt(1));
    auto m1 = std::make_shared<SingleMachine>(SingleMachine());
    auto m2 = std::make_shared<SingleMachine>(SingleMachine());
    auto m3 = std::make_shared<SingleMachine>(SingleMachine());

    w.addEntity(mg);
    w.addEntity(belt1);
    w.addEntity(belt2);
    w.addEntity(belt3);
    w.addEntity(m1);
    w.addEntity(m2);
    w.addEntity(m3);

    belt1->connectInput(0, m1, 0);
    belt2->connectInput(0, m2, 0);
    mg->connectInput(0, belt1, 0);
    mg->connectInput(1, belt2, 0);
    belt3->connectInput(0, mg, 0);
    m3->connectInput(0, belt3, 0);

    m1->getOutputStack(0)->addAmount(1, Resource::Iron_Ore);
    m2->getOutputStack(0)->addAmount(1, Resource::Iron_Ore);

    w.advanceBy(4 * 1000, [&]() {
        EXPECT_EQ(m3->getInputStack(0)->getAmount(), 2);
        EXPECT_EQ(m1->getOutputStack(0)->getAmount(), 0);
        EXPECT_EQ(m2->getOutputStack(0)->getAmount(), 0);
        EXPECT_EQ(belt1->_in_transit_stack.size(), 0);
        EXPECT_EQ(belt2->_in_transit_stack.size(), 0);
        EXPECT_EQ(belt3->_in_transit_stack.size(), 0);
        EXPECT_EQ(mg->_in_transit_stack.size(), 0);
    });
}