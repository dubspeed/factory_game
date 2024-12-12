#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(SingleMachineTests, CheckAndStartProcessing) {
    auto m = SingleMachine();
    auto r = SingleRecipe{Resource::Iron_Ore, 5, Resource::Iron_Ingots, 1, 4};
    m._checkAndStartProcessing();
    EXPECT_EQ(m.processing, false);
    m.setRecipe(r);
    m.getInputStack(0)->addAmount(5, r.r_in);
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
    auto r = SingleRecipe{Resource::Iron_Ore, 5, Resource::Iron_Ingots, 1, 4};
    EXPECT_EQ(m._canStartProduction(), false);
    m.setRecipe(r);
    EXPECT_FALSE(m._canStartProduction());
    m.getInputStack(0)->addAmount(5, r.r_in);
    EXPECT_TRUE(m._canStartProduction());
    m.getOutputStack(0)->addAmount(MAX_STACK_SIZE, r.r_out);
    EXPECT_FALSE(m._canStartProduction());
    m.getOutputStack(0)->clear();
    m.processing_progress = r.processing_time_s * 1000;
    EXPECT_EQ(m._canStartProduction(), true);
}