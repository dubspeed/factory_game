#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Storage, CanStoreOneItem) {
    auto w = GameWorld();
    const auto m = std::make_shared<SingleMachine>(SingleMachine());
    const auto b = std::make_shared<Belt>(Belt(1));
    const auto s = std::make_shared<Storage>(Storage());
    s->setMaxItemStacks(1);

    w.addEntity(m);
    w.addEntity(b);
    w.addEntity(s);

    m->getOutputStack(0)->addAmount(1, Resource::Iron_Ore);
    b->connectInput(0, m, 0);
    s->connectInput(0, b, 0);

    w.advanceBy(2 * 1000, [&]() {
        EXPECT_EQ(m->getOutputStack(0)->getAmount(), 0);
        EXPECT_EQ(b->_in_transit_stack.size(), 0);
        EXPECT_EQ(s->getInputStack(0)->getAmount(), 0);
        EXPECT_EQ(s->getAmount(Resource::Iron_Ore), 1);
    });
}

TEST(Storage, CanStoreMultipleItems) {
    auto w = GameWorld();
    const auto m = std::make_shared<SingleMachine>(SingleMachine());
    const auto b = std::make_shared<Belt>(Belt(1));
    const auto s = std::make_shared<Storage>(Storage());
    s->setMaxItemStacks(2);

    w.addEntity(m);
    w.addEntity(b);
    w.addEntity(s);

    m->getOutputStack(0)->addAmount(1, Resource::Iron_Ore);
    b->connectInput(0, m, 0);
    s->connectInput(0, b, 0);

    w.advanceBy(2 * 1000, [&]() {
        EXPECT_EQ(s->getAmount(Resource::Iron_Ore), 1);
    });
    m->getOutputStack(0)->addAmount(1, Resource::Copper_Cable);
    w.advanceBy(2 * 1000, [&]() {
        EXPECT_FALSE(b->getJammed());
        EXPECT_EQ(s->getAmount(Resource::Copper_Cable), 1);
    });
}

TEST(Storage, CanNotStoreMoreWhenFull) {
    auto w = GameWorld();
    const auto s = std::make_shared<Storage>(Storage());
    s->setMaxItemStacks(1);
    w.addEntity(s);

    s->getInputStack(0)->addAmount(MAX_STACK_SIZE, Resource::Iron_Ore);

    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::Iron_Ore), MAX_STACK_SIZE);
    });
    // now we can't store more

    s->getInputStack(0)->addAmount(1, Resource::Iron_Ore);
    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::Iron_Ore), MAX_STACK_SIZE);
    });

    s->getInputStack(0)->addAmount(1, Resource::Copper_Cable);
    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::Copper_Cable), 0);
    });
}

TEST(Storage, CanStoreMoreStacksOfTheSameItem) {
    auto w = GameWorld();
    const auto s = std::make_shared<Storage>(Storage());
    s->setMaxItemStacks(2);
    w.addEntity(s);

    s->getInputStack(0)->addAmount(MAX_STACK_SIZE, Resource::Iron_Ore);

    // The input stack will be consumed 1 per frame
    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::Iron_Ore), MAX_STACK_SIZE);
    });

    s->getInputStack(0)->addAmount(MAX_STACK_SIZE, Resource::Iron_Ore);
    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::Iron_Ore), MAX_STACK_SIZE * 2);
    });
}
