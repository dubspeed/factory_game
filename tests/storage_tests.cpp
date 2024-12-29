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

    // the storage can hold some items in it's output stack
    // in addition to the content stacks
    constexpr static int MAX_STORAGE_CAPACITY = MAX_STACK_SIZE + Storage::OUTPUT_STACK_SIZE;
    s->getInputStack(0)->addAmount(MAX_STACK_SIZE, Resource::IronOre);

    // it's a bit tricky to fill the storage, because we can only set max MAX_STACK_SIZE items
    // in the input stack, and the output stack can hold some items too
    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::IronOre), MAX_STACK_SIZE);
    });
    // now we can store 5 more
    s->getInputStack(0)->addAmount(5, Resource::IronOre);
    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::IronOre), MAX_STORAGE_CAPACITY);
    });

    // now we can't store more
    s->getInputStack(0)->addAmount(5, Resource::IronOre);
    w.advanceBy(100, [&]() {
        EXPECT_EQ(s->getAmount(Resource::IronOre), MAX_STORAGE_CAPACITY);
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

TEST(Storage, CanFeedOutputConnectionWithItems) {
    auto w = Factory();
    const auto s = std::make_shared<Storage>(Storage());
    const auto m = std::make_shared<Machine>(Machine());
    const auto b = std::make_shared<Belt>(Belt(1));
    s->setMaxItemStacks(1);
    w.addEntity(s);
    w.addEntity(m);
    w.addEntity(b);
    m->setRecipe(recipe_IronIngot);

    s->manualAdd(1, Resource::IronOre);
    b->connectInput(0, s, 0);
    m->connectInput(0, b, 0);

    w.advanceBy(1200, [&]() {
        EXPECT_TRUE(m->processing);
        EXPECT_EQ(s->getAmount(Resource::IronOre), 0);
        EXPECT_FALSE(b->getActive());

    });
    w.advanceBy(2 * 1000, [&]() {
        EXPECT_EQ(m->getOutputStack(0)->getAmount(), 1);
    });
}