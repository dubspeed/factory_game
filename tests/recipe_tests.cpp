#include "gtest/gtest.h"
#include "../src/factory.h"
using namespace Fac;

TEST(RecipeTests, Recipe) {
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };
    EXPECT_EQ(r.inputs[0].amount, 5);
    EXPECT_EQ(r.products[0].amount, 1);
    EXPECT_EQ(r.processing_time_s, 4);
    EXPECT_EQ(r.inputs[0].resource, Resource::IronOre);
    EXPECT_EQ(r.products[0].resource, Resource::IronIngot);
}

TEST(RecipeTests, InAMachine) {
    auto r = recipe_iron_ingots;
    auto m = SingleMachine();
    m.setRecipe(r);
    EXPECT_EQ(m.getOutputRpm(), 30);
    EXPECT_EQ(m.getInputRpm(), 30);
}

TEST(RecipeTests, SimpleProductionCheckWithTime) {
    auto w = GameWorld();
    // Setup a single smelter
    const auto m = std::make_shared<SingleMachine>(SingleMachine());
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };
    w.addEntity(m);
    m->setRecipe(r);
    m->getInputStack(0)->addAmount(5, r.inputs[0].resource);;
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

    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };

    // Register the machine in the simulation
    w.addEntity(m);

    // set a recipe
    m->setRecipe(r);

    // add some raw material
    m->getInputStack(0)->addAmount(MAX_STACK_SIZE, r.inputs[0].resource);;

    // move time to the future, by iterating via delta_t
    for (auto i = 0; i < 12001; i++) {
        w.update(1);
        if (i % 4000 == 0) {
            // std::cout << "Time: " << i << std::endl;
            if (i > 0)
                EXPECT_EQ(m->getInputStack(0)->getAmount(), MAX_STACK_SIZE - r.inputs[0].amount * (i / 4000 + 1));
            if (i == 4000)
                EXPECT_EQ(m->getOutputStack(0)->getAmount(), 1);
            if (i == 8000)
                EXPECT_EQ(m->getOutputStack(0)->getAmount(), 2);
            if (i == 12000)
                EXPECT_EQ(m->getOutputStack(0)->getAmount(), 3);
        }
    }
}
