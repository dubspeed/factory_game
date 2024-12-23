#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Storage, CanStoreOneItem) {
    auto w = Factory();
    const auto m = std::make_shared<Machine>(Machine());
    const auto b = std::make_shared<Belt>(Belt(1));
    const auto s = std::make_shared<Storage>(Storage());
    s->setMaxItemStacks(1);

    w.addEntity(m);
    w.addEntity(b);
    w.addEntity(s);

    m->getOutputStack(0)->addAmount(1, Resource::IronOre);
    b->connectInput(0, m, 0);
    s->connectInput(0, b, 0);

    w.advanceBy(2 * 1000, [&]() {
        EXPECT_EQ(m->getOutputStack(0)->getAmount(), 0);
        EXPECT_EQ(b->_in_transit_stack.size(), 0);
        EXPECT_EQ(s->getInputStack(0)->getAmount(), 0);
        EXPECT_EQ(s->getAmount(Resource::IronOre), 1);
    });
}

TEST(Storage, CanStoreMultipleItems) {
    auto w = Factory();
    const auto m = std::make_shared<Machine>(Machine());
    const auto b = std::make_shared<Belt>(Belt(1));
    const auto s = std::make_shared<Storage>(Storage());
    s->setMaxItemStacks(2);

    w.addEntity(m);
    w.addEntity(b);
    w.addEntity(s);

    m->getOutputStack(0)->addAmount(1, Resource::IronOre);
    b->connectInput(0, m, 0);
    s->connectInput(0, b, 0);

    w.advanceBy(2 * 1000, [&]() {
        EXPECT_EQ(s->getAmount(Resource::IronOre), 1);
    });
    m->getOutputStack(0)->addAmount(1, Resource::Cable);
    w.advanceBy(2 * 1000, [&]() {
        EXPECT_FALSE(b->getJammed());
        EXPECT_EQ(s->getAmount(Resource::Cable), 1);
    });
}

TEST(Storage, CanNotStoreMoreWhenFull) {
    auto w = Factory();
    const auto s = std::make_shared<Storage>(Storage());
    s->setMaxItemStacks(1);
    w.addEntity(s);

    s->getInputStack(0)->addAmount(MAX_STACK_SIZE, Resource::IronOre);

    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::IronOre), MAX_STACK_SIZE);
    });
    // now we can't store more

    s->getInputStack(0)->addAmount(1, Resource::IronOre);
    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::IronOre), MAX_STACK_SIZE);
    });

    s->getInputStack(0)->addAmount(1, Resource::Cable);
    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::Cable), 0);
    });
}

TEST(Storage, CanStoreMoreStacksOfTheSameItem) {
    auto w = Factory();
    const auto s = std::make_shared<Storage>(Storage());
    s->setMaxItemStacks(2);
    w.addEntity(s);

    s->getInputStack(0)->addAmount(MAX_STACK_SIZE, Resource::IronOre);

    // The input stack will be consumed 1 per frame
    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::IronOre), MAX_STACK_SIZE);
    });

    s->getInputStack(0)->addAmount(MAX_STACK_SIZE, Resource::IronOre);
    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::IronOre), MAX_STACK_SIZE * 2);
    });
}
