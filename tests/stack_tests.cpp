#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Stack, CanBeUsed) {
    auto s = Stack();
    EXPECT_TRUE(s.isEmpty());
    EXPECT_FALSE(s.isFull());
    EXPECT_TRUE(s.canAdd(1, Resource::Iron_Ore));
    EXPECT_FALSE(s.canAdd(101, Resource::Iron_Ore));
    s.addOne(Resource::Iron_Ore);
    EXPECT_FALSE(s.isEmpty());
    EXPECT_FALSE(s.isFull());
    EXPECT_TRUE(s.canAdd(1, Resource::Iron_Ore));
    EXPECT_FALSE(s.canAdd(101, Resource::Iron_Ore));
    s.addAmount(MAX_STACK_SIZE - s.getAmount(), Resource::Iron_Ore);
    EXPECT_FALSE(s.isEmpty());
    EXPECT_TRUE(s.isFull());
    EXPECT_FALSE(s.canAdd(1, Resource::Iron_Ore));
    EXPECT_FALSE(s.canAdd(101, Resource::Iron_Ore));
    s.removeOne();
    EXPECT_FALSE(s.isEmpty());
    EXPECT_FALSE(s.isFull());
    EXPECT_TRUE(s.canAdd(1, Resource::Iron_Ore));
    EXPECT_FALSE(s.canAdd(101, Resource::Iron_Ore));
    s.removeAmount(99);
    EXPECT_TRUE(s.isEmpty());
    EXPECT_FALSE(s.isFull());
    EXPECT_TRUE(s.canAdd(1, Resource::Iron_Ore));
    EXPECT_FALSE(s.canAdd(101, Resource::Iron_Ore));
}

TEST(Stack, CanBeCleared) {
    auto s = Stack();
    s.addAmount(10, Resource::Iron_Ore);
    EXPECT_FALSE(s.isEmpty());
    s.clear();
    EXPECT_TRUE(s.isEmpty());
}

TEST(Stack, CanSetDifferentResourceWhenEmpty) {
    auto s = Stack();
    s.addAmount(10, Resource::Iron_Ore);
    EXPECT_FALSE(s.isEmpty());
    s.clear();
    s.addAmount(10, Resource::Copper_Ore);
    EXPECT_FALSE(s.isEmpty());
    EXPECT_EQ(s.getResource(), Resource::Copper_Ore);
}
