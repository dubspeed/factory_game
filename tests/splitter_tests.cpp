#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Splitter, ShouldDoSomeSplitting) {
    auto w = Factory();
    auto sp = std::make_shared<Splitter>(Splitter());

    // setup: a machine that produces iron ingots
    // and two machines that produce iron plates
    // connected via a splitter
    auto m1 = std::make_shared<Machine>(Machine());
    auto m2 = std::make_shared<Machine>(Machine());
    auto m3 = std::make_shared<Machine>(Machine());

    m1->setRecipe(recipe_IronIngot);
    m1->getInputStack(0)->addAmount(2, recipe_IronIngot.inputs[0].resource);
    m2->setRecipe(recipe_IronPlate);
    m3->setRecipe(recipe_IronPlate);

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
    // + 3 frames for the belt to move the item
    w.advanceBy(2000 + 3003, [&]() {
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

TEST(Splitter, HasAOutputStackOfOneItem) {
    auto sp = std::make_shared<Splitter>();
    sp->getOutputStack(0)->setMaxStackSize(1);
    sp->getOutputStack(0)->addAmount(1, Resource::IronOre);

    EXPECT_EQ(sp->getOutputStack(0)->getAmount(), 1);
    EXPECT_FALSE(sp->getOutputStack(0)->canAdd(1, Resource::IronOre));
    EXPECT_TRUE(sp->getOutputStack(0)->isFull());
}
