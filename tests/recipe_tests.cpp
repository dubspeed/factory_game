#include "gtest/gtest.h"
#include "../src/factory.h"
using namespace Fac;

TEST(RecipeTests, SingleRecipe) {
    auto r = SingleRecipe{Resource::Iron_Ore, 5, Resource::Iron_Ingots, 1, 4};
    EXPECT_EQ(r.amount_in, 5);
    EXPECT_EQ(r.amount_out, 1);
    EXPECT_EQ(r.processing_time_s, 4);
    EXPECT_EQ(r.r_in, Resource::Iron_Ore);
    EXPECT_EQ(r.r_out, Resource::Iron_Ingots);
}

TEST(RecipeTests, InAMachine) {
    auto r = SingleRecipe{Resource::Iron_Ore, 5, Resource::Iron_Ingots, 1, 4};
    auto m = SingleMachine();
    m.setRecipe(r);
    EXPECT_EQ(m.getOutputRpm(), 15);
    EXPECT_EQ(m.getInputRpm(), 75);
}

TEST(RecipeTest, SimpleProductionCheckWithTime) {
    auto w = GameWorld();
    // Setup a single smelter
    const auto m = std::make_shared<SingleMachine>(SingleMachine());
    auto r = SingleRecipe{Resource::Iron_Ore, 5, Resource::Iron_Ingots, 1, 4};
    w.addEntity(m);
    m->setRecipe(r);
    m->getInputStack(0)->addAmount(5, r.r_in);
    for (auto i = 0; i < 4; i++) {
        w.update(1000);
        EXPECT_EQ(m->processing, true);
        EXPECT_TRUE(m->getInputStack(0)->isEmpty());
        EXPECT_TRUE(m->getOutputStack(0)->isEmpty());
        EXPECT_EQ(m->processing_progress, 1000 * (i));
    }
    w.update(1000);
    EXPECT_EQ(m->processing, false);
    EXPECT_TRUE(m->getInputStack(0)->isEmpty());
    EXPECT_EQ(m->getOutputStack(0)->getAmount(), 1);
    EXPECT_EQ(m->processing_progress, 4000);
}

TEST(RecipeTests, LetsPassTime) {
    auto w = GameWorld();
    // Setup a single smelter
    const auto m = std::make_shared<SingleMachine>(SingleMachine());

    auto r = SingleRecipe{Resource::Iron_Ore, 5, Resource::Iron_Ingots, 1, 4};

    // Register the machine in the simulation
    w.addEntity(m);

    // set a recipe
    m->setRecipe(r);

    // add some raw material
    m->getInputStack(0)->addAmount(MAX_STACK_SIZE, r.r_in);

    // move time to the future, by iterating via delta_t
    for (auto i = 0; i < 12001; i++) {
        w.update(1);
        if (i % 4000 == 0) {
            // std::cout << "Time: " << i << std::endl;
            if (i > 0)
                EXPECT_EQ(m->getInputStack(0)->getAmount(), MAX_STACK_SIZE - r.amount_in * (i / 4000 + 1));
            if (i == 4000)
                EXPECT_EQ(m->getOutputStack(0)->getAmount(), 1);
            if (i == 8000)
                EXPECT_EQ(m->getOutputStack(0)->getAmount(), 2);
            if (i == 12000)
                EXPECT_EQ(m->getOutputStack(0)->getAmount(), 3);
        }
    }
}
