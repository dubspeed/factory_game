#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Removal, CanRemoveAGameworldEntity) {
    auto f = Factory();
    const auto m = std::make_shared<Machine>(Machine());
    f.addEntity(m);
    f.removeEntity(m);
    EXPECT_EQ(f.getEntities().size(), 0);
}

TEST(Removal, RemovesALinkedEntity) {
    auto f = Factory();
    const auto m = std::make_shared<Machine>(Machine());
    const auto b = std::make_shared<Belt>(Belt(1));
    f.addEntity(m);
    f.addEntity(b);
    m->connectInput(0, b, 0);
    f.removeEntity(m);
    EXPECT_EQ(f.getEntities().size(), 1);
    EXPECT_EQ(f.getEntities()[0]->getId(), b->getId());
}

TEST(Removal, RemoveAnEntityInTheMiddleOfAChain) {
    auto f = Factory();
    const auto m1 = std::make_shared<Machine>(Machine());
    const auto m2 = std::make_shared<Machine>(Machine());
    const auto b = std::make_shared<Belt>(Belt(1));
    m1->setRecipe(recipe_IronIngot);
    m2->setRecipe(recipe_IronPlate);
    m1->getInputStack(0)->addAmount(MAX_STACK_SIZE, Resource::IronOre);
    f.addEntity(m1);
    f.addEntity(m2);
    f.addEntity(b);
    b->connectInput(0, m1, 0);
    m2->connectInput(0, b, 0);
    f.advanceBy(20 * 1000, [&]() {
        EXPECT_EQ(m1->getOutputStack(0)->getAmount(), 0);
        EXPECT_EQ(m2->getOutputStack(0)->getAmount(), 4);
        EXPECT_EQ(m2->getInputStack(0)->getAmount(), 0);
    });
    f.removeEntity(b);
    EXPECT_EQ(f.getEntities().size(), 2);
    EXPECT_EQ(f.getEntities()[0]->getId(), m1->getId());
    EXPECT_EQ(f.getEntities()[1]->getId(), m2->getId());
    // after removal of the belt, both machines continue to work
    // but get no items fed
    f.advanceBy(20 * 1000, [&]() {
        EXPECT_EQ(m1->getOutputStack(0)->getAmount(), 10);
        EXPECT_EQ(m2->getOutputStack(0)->getAmount(), 6);
        EXPECT_EQ(m2->getInputStack(0)->getAmount(), 0);
    });
}

TEST(Removal, DoesNotAffectSerialzation) {
    auto f = Factory();
    const auto m1 = std::make_shared<Machine>(Machine());
    const auto b = std::make_shared<Belt>(Belt(1));
    const auto m2 = std::make_shared<Machine>(Machine());
    m1->setRecipe(recipe_IronIngot);
    m2->setRecipe(recipe_IronPlate);
    m1->getInputStack(0)->addAmount(MAX_STACK_SIZE, Resource::IronOre);
    f.addEntity(m1);
    f.addEntity(b);
    f.addEntity(m2);
    b->connectInput(0, m1, 0);
    m2->connectInput(0, b, 0);
    f.advanceBy(20 * 1000, [&]() {
        EXPECT_EQ(m1->getOutputStack(0)->getAmount(), 0);
        EXPECT_EQ(m2->getOutputStack(0)->getAmount(), 4);
        EXPECT_EQ(m2->getInputStack(0)->getAmount(), 0);
    });
    f.removeEntity(b);

    json j = f;
    const auto f2 = j.get<Factory>();
    const auto m1_f2 = std::dynamic_pointer_cast<Machine>(f2.getEntityById(m1->getId()).value());
    const auto m2_f2 = std::dynamic_pointer_cast<Machine>(f2.getEntityById(m2->getId()).value());

    // after removal of the belt, both machines continue to work a bit
    f2.advanceBy(20 * 1000, [&]() {
        EXPECT_EQ(m1_f2->getOutputStack(0)->getAmount(), 10);
        EXPECT_EQ(m2_f2->getOutputStack(0)->getAmount(), 6);
        EXPECT_EQ(m2_f2->getInputStack(0)->getAmount(), 0);
    });
}