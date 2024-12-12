#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Splitter, ShouldDoSomeSplitting) {
    auto w = GameWorld();
    auto sp = std::make_shared<Splitter>(Splitter());

    auto m1 = std::make_shared<SingleMachine>(SingleMachine());
    auto m2 = std::make_shared<SingleMachine>(SingleMachine());
    auto m3 = std::make_shared<SingleMachine>(SingleMachine());

    m1->setRecipe(recipe_iron_ingots);
    m1->getInputStack(0)->addAmount(2, recipe_iron_ingots.r_in);
    m2->setRecipe(recipe_iron_plates);
    m3->setRecipe(recipe_iron_plates);

    auto belt1 = std::make_shared<Belt>(Belt(1));
    auto belt2 = std::make_shared<Belt>(Belt(1));
    auto belt3 = std::make_shared<Belt>(Belt(1));

    belt1->connectInput(0, m1, 0);
    sp->connectInput(0, belt1, 0);
    belt2->connectInput(0, sp, 0);
    belt3->connectInput(0, sp, 1);
    m2->connectInput(0, belt2, 0);
    m3->connectInput(0, belt3, 0);

    w.addEntity(m1);
    w.addEntity(m2);
    w.addEntity(m3);
    w.addEntity(sp);
    w.addEntity(belt1);
    w.addEntity(belt2);
    w.addEntity(belt3);

    // the first takes 2 seconds to produces and 3 seconds to
    // pass belt1, belt2 and end up in m2
    w.advanceBy(2000 + 3000, [&]() {
        EXPECT_EQ(m1->getInputStack(0)->getAmount(), 0);
        EXPECT_EQ(m2->getInputStack(0)->getAmount(), 1);
        EXPECT_EQ(m3->getInputStack(0)->getAmount(), 0);
        EXPECT_FALSE(belt1->getJammed());
        EXPECT_FALSE(belt2->getJammed());
        EXPECT_FALSE(belt3->getJammed());
        EXPECT_FALSE(sp->getJammed());
    });

    w.advanceBy(2000 + 2000 + 3000, [&]() {
        EXPECT_EQ(m1->getInputStack(0)->getAmount(), 0);
        EXPECT_EQ(m2->getInputStack(0)->getAmount(), 1);
        EXPECT_EQ(m3->getInputStack(0)->getAmount(), 1);
        EXPECT_FALSE(belt1->getJammed());
        EXPECT_FALSE(belt2->getJammed());
        EXPECT_FALSE(belt3->getJammed());
        EXPECT_FALSE(sp->getJammed());
    });
}