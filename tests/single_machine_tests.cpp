#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(SingleMachineTests, CheckAndStartProcessing) {
    auto m = SingleMachine();
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };
    m._checkAndStartProcessing();
    EXPECT_EQ(m.processing, false);
    m.setRecipe(r);
    m.getInputStack(0)->addAmount(5, r.inputs[0].resource);
    m.processing_progress = 1.0;
    m.processing = false;
    m._checkAndStartProcessing();
    EXPECT_EQ(m.processing, true);
    EXPECT_TRUE(m.getInputStack(0)->isEmpty());
    EXPECT_EQ(m.processing_progress, 0);
    // second call will not change anything
    m._checkAndStartProcessing();
    EXPECT_EQ(m.processing, true);
    EXPECT_TRUE(m.getInputStack(0)->isEmpty());
    EXPECT_EQ(m.processing_progress, 0);
}

TEST(SingleMachineTests, CanProduce) {
    auto m = SingleMachine();
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };
    EXPECT_EQ(m._canStartProduction(), false);
    m.setRecipe(r);
    EXPECT_FALSE(m._canStartProduction());
    m.getInputStack(0)->addAmount(5, r.inputs[0].resource);
    EXPECT_TRUE(m._canStartProduction());
    m.getOutputStack(0)->addAmount(MAX_STACK_SIZE, r.products[0].resource);
    EXPECT_FALSE(m._canStartProduction());
    m.getOutputStack(0)->clear();
    m.processing_progress = r.processing_time_s * 1000;
    EXPECT_EQ(m._canStartProduction(), true);
}