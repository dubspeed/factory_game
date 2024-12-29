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
    auto m = Machine();
    m.setRecipe(recipe_IronIngot);
    EXPECT_EQ(m.getOutputRpm(), 30);
    EXPECT_EQ(m.getInputRpm(), 30);
}

TEST(RecipeTests, SimpleProductionCheckWithTime) {
    auto w = Factory();
    // Setup a single smelter
    const auto m = std::make_shared<Machine>(Machine());
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };
    w.addEntity(m);
    m->setRecipe(r);
    m->getFirstInput()->addAmount(5, r.inputs[0].resource);;
    for (auto i = 0; i < 4; i++) {
        w.update(999);
        EXPECT_EQ(m->processing, true);
        EXPECT_TRUE(m->getFirstInput()->isEmpty());
        EXPECT_TRUE(m->getOutputStack(0)->isEmpty());
        EXPECT_EQ(m->processing_progress, 999 * i);
    }
    w.update(1010);
    EXPECT_EQ(m->processing, false);
    EXPECT_TRUE(m->getFirstInput()->isEmpty());
    EXPECT_EQ(m->getOutputStack(0)->getAmount(), 1);
    EXPECT_EQ(m->processing_progress, 0);
}

TEST(RecipeTests, LetsPassTime) {
    auto w = Factory();
    // Setup a single smelter
    const auto m = std::make_shared<Machine>(Machine());

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
    m->getFirstInput()->addAmount(MAX_STACK_SIZE, r.inputs[0].resource);;

    auto const time = r.processing_time_s * 1000;

    auto in = m->getFirstInput();
    // move time to the future, by iterating via delta_t
    // the +10 everywhere are just to make sure we are not missing any
    // stuff by adding some extra frame for processing
    for (auto i = 0; i <= time * 3 + 10; i++) {
        w.update(1);
        if (i + 10 % time + 10 == 0) {
            std::cout << "Time: " << i << std::endl;
            if (i > 0)
                EXPECT_EQ(in->getAmount(), MAX_STACK_SIZE - r.inputs[0].amount * (i / time));
            if (i == time + 10)
                EXPECT_EQ(m->getOutputStack(0)->getAmount(), 1);
            if (i == time * 2 + 10)
                EXPECT_EQ(m->getOutputStack(0)->getAmount(), 2);
            if (i == time * 3 + 10)
                EXPECT_EQ(m->getOutputStack(0)->getAmount(), 3);
        }
    }
}
