#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Stack, CanBeUsed) {
    auto s = Stack();
    EXPECT_TRUE(s.isEmpty());
    EXPECT_FALSE(s.isFull());
    EXPECT_TRUE(s.canAdd(1, Resource::IronOre));
    EXPECT_FALSE(s.canAdd(101, Resource::IronOre));
    s.addOne(Resource::IronOre);
    EXPECT_FALSE(s.isEmpty());
    EXPECT_FALSE(s.isFull());
    EXPECT_TRUE(s.canAdd(1, Resource::IronOre));
    EXPECT_FALSE(s.canAdd(101, Resource::IronOre));
    s.addAmount(MAX_STACK_SIZE - s.getAmount(), Resource::IronOre);
    EXPECT_FALSE(s.isEmpty());
    EXPECT_TRUE(s.isFull());
    EXPECT_FALSE(s.canAdd(1, Resource::IronOre));
    EXPECT_FALSE(s.canAdd(101, Resource::IronOre));
    s.removeOne();
    EXPECT_FALSE(s.isEmpty());
    EXPECT_FALSE(s.isFull());
    EXPECT_TRUE(s.canAdd(1, Resource::IronOre));
    EXPECT_FALSE(s.canAdd(101, Resource::IronOre));
    s.removeAmount(99);
    EXPECT_TRUE(s.isEmpty());
    EXPECT_FALSE(s.isFull());
    EXPECT_TRUE(s.canAdd(1, Resource::IronOre));
    EXPECT_FALSE(s.canAdd(101, Resource::IronOre));
}

TEST(Stack, CanBeCleared) {
    auto s = Stack();
    s.addAmount(10, Resource::IronOre);
    EXPECT_FALSE(s.isEmpty());
    s.clear();
    EXPECT_TRUE(s.isEmpty());
}

TEST(Stack, CanSetDifferentResourceWhenEmpty) {
    auto s = Stack();
    s.addAmount(10, Resource::IronOre);
    EXPECT_FALSE(s.isEmpty());
    s.clear();
    s.addAmount(10, Resource::CopperOre);
    EXPECT_FALSE(s.isEmpty());
    EXPECT_EQ(s.getResource(), Resource::CopperOre);
}

TEST(Stack, CanNotSetDifferentResourceWhenNotEmpty) {
    auto s = Stack();
    s.addAmount(10, Resource::IronOre);
    EXPECT_FALSE(s.isEmpty());
    s.addAmount(10, Resource::CopperOre);
    EXPECT_FALSE(s.isEmpty());
    EXPECT_EQ(s.getResource(), Resource::IronOre);
}

TEST(Stack, ReturnsNoneResourceWhenEmpty) {
    auto s = Stack();
    EXPECT_TRUE(s.isEmpty());
    EXPECT_EQ(s.getResource(), Resource::None);
    s.addAmount(10, Resource::IronOre);
    EXPECT_FALSE(s.isEmpty());
    EXPECT_EQ(s.getResource(), Resource::IronOre);
}