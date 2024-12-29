#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Machine, CanProduce) {
    auto m = Machine();
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };
    EXPECT_EQ(m.canStartProduction(), false);
    m.setRecipe(r);
    EXPECT_FALSE(m.canStartProduction());
    m.getFirstInput()->addAmount(5, r.inputs[0].resource);
    EXPECT_TRUE(m.canStartProduction());
    m.getOutputStack(0)->addAmount(MAX_STACK_SIZE, r.products[0].resource);
    EXPECT_FALSE(m.canStartProduction());
    m.getOutputStack(0)->clear();
    m.processing_progress = r.processing_time_s * 1000;
    EXPECT_EQ(m.canStartProduction(), true);
}

TEST(Machine, HasAInputStack) {
    auto m = std::make_shared<Machine>();
    auto s = m->getInputStack(0);
    EXPECT_TRUE(s->isEmpty());
    s->addAmount(MAX_STACK_SIZE, Resource::IronOre);
    EXPECT_EQ(s->getAmount(), MAX_STACK_SIZE);
    EXPECT_EQ(s->getResource(), Resource::IronOre);

    // Link machine to a belt and verify the stack has not changed
    auto b = std::make_shared<Belt>(1);
    m->connectInput(0, b, 0);
    EXPECT_EQ(s->getAmount(), MAX_STACK_SIZE);
    EXPECT_EQ(s->getResource(), Resource::IronOre);
}

TEST(Machine, HasAInputStackIndipendentOfConnectedOutput) {
    auto m = std::make_shared<Machine>();
    auto b = std::make_shared<Belt>(1);
    auto s = m->getInputStack(0);
    EXPECT_TRUE(s->isEmpty());
    s->addAmount(MAX_STACK_SIZE, Resource::IronOre);
    EXPECT_EQ(s->getAmount(), MAX_STACK_SIZE);
    EXPECT_EQ(s->getResource(), Resource::IronOre);
    EXPECT_TRUE(b->getOutputStack(0)->isEmpty());
    b->getOutputStack(0)->addAmount(1, Resource::IronIngot);
    EXPECT_EQ(s->getAmount(), MAX_STACK_SIZE);
    EXPECT_EQ(b->getOutputStack(0)->getAmount(), 1);
}

TEST(Machine, SetRecipe) {
    auto m = std::make_shared<Machine>();
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}},
        .products = {{Resource::IronIngot, 1}},
        .processing_time_s = 4
    };

    EXPECT_FALSE(m->getRecipe().has_value());
    m->setRecipe(r);
    EXPECT_TRUE(m->getRecipe().has_value());
    EXPECT_EQ(m->getRecipe().value().inputs[0].amount, 5);
    EXPECT_EQ(m->getRecipe().value().products[0].amount, 1);
    EXPECT_EQ(m->getRecipe().value().processing_time_s, 4);
    EXPECT_EQ(m->getRecipe().value().inputs[0].resource, Resource::IronOre);
    EXPECT_EQ(m->getRecipe().value().products[0].resource, Resource::IronIngot);
    EXPECT_EQ(m->getInputRpm(), 75);
    EXPECT_EQ(m->getOutputRpm(), 15);
    EXPECT_EQ(m->getOutputStack(0)->getResource(), Resource::IronIngot);
    EXPECT_EQ(m->getOutputStack(0)->getAmount(), 0);
    EXPECT_EQ(m->getInputStack(0)->getResource(), Resource::IronOre);
    EXPECT_EQ(m->getInputStack(0)->getAmount(), 0);
}

TEST(Machine, MachineWithTwoInputResources) {
    auto f = Factory();
    auto m = std::make_shared<Machine>(2, 1);
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}, {Resource::CopperOre, 3}},
        .products = {{Resource::SmartPlating, 1}},
        .processing_time_s = 4
    };
    f.addEntity(m);
    m->setRecipe(r);
    m->getFirstInput()->addAmount(5, Resource::IronOre);
    m->getSecondInput()->addAmount(3, Resource::CopperOre);
    EXPECT_EQ(m->getFirstInput()->getAmount(), 5);
    EXPECT_EQ(m->getSecondInput()->getAmount(), 3);
    EXPECT_EQ(m->getInputRpm(), 60);
    EXPECT_EQ(m->getOutputRpm(), 15);

    f.advanceBy(4 * 1000, [&]() {
        EXPECT_EQ(m->getFirstInput()->getAmount(), 0);
        EXPECT_EQ(m->getSecondInput()->getAmount(), 0);
        EXPECT_EQ(m->getOutputStack(0)->getAmount(), 1);
    });
}

TEST(Machine, AssemblerSetup) {
    auto f = Factory();
    auto m = std::make_shared<Machine>(2, 1);
    m->setRecipe(recipe_ReinforcedIronPlate);
    auto s1 = std::make_shared<Storage>();
    s1->manualAdd(MAX_STACK_SIZE, Resource::IronPlate);
    auto s2 = std::make_shared<Storage>();
    s2->manualAdd(MAX_STACK_SIZE, Resource::Screw);
    auto b1 = std::make_shared<Belt>(100);
    auto b2 = std::make_shared<Belt>(100);
    f.addEntity(m);
    f.addEntity(s1);
    f.addEntity(s2);
    f.addEntity(b1);
    f.addEntity(b2);
    b1->connectInput(0, s1, 0);
    b2->connectInput(0, s2, 0);
    m->connectInput(0, b1, 0);
    m->connectInput(1, b2, 0);

    // 300 ms overlay for belts and movement if items inside the storage container
    f.advanceBy(12 * 1000 + 300, [&]() {
        EXPECT_EQ(s1->getAmount(Resource::IronPlate), 0);
        EXPECT_EQ(s2->getAmount(Resource::Screw), 0);
        EXPECT_EQ(m->getOutputStack(0)->getAmount(), 1);
    });
}

TEST(Machine, BigMachineWithByProducts) {
    // machine with 4 inputs and 2 outputs, feed by 4 storages
    auto f = Factory();
    auto m = std::make_shared<Machine>(4, 2);
    Recipe r = {
        .inputs = {{Resource::IronOre, 5}, {Resource::CopperOre, 3}, {Resource::Coal, 2}, {Resource::Limestone, 1}},
        .products = {{Resource::SteelIngot, 1}, {Resource::Concrete, 1}},
        .processing_time_s = 4
    };
    f.addEntity(m);
    auto s1 = std::make_shared<Storage>();
    s1->manualAdd(MAX_STACK_SIZE, Resource::IronOre);
    auto s2 = std::make_shared<Storage>();
    s2->manualAdd(MAX_STACK_SIZE, Resource::CopperOre);
    auto s3 = std::make_shared<Storage>();
    s3->manualAdd(MAX_STACK_SIZE, Resource::Coal);
    auto s4 = std::make_shared<Storage>();
    s4->manualAdd(MAX_STACK_SIZE, Resource::Limestone);
    auto b1 = std::make_shared<Belt>(100);
    auto b2 = std::make_shared<Belt>(100);
    auto b3 = std::make_shared<Belt>(100);
    auto b4 = std::make_shared<Belt>(100);
    f.addEntity(s1);
    f.addEntity(s2);
    f.addEntity(s3);
    f.addEntity(s4);
    f.addEntity(b1);
    f.addEntity(b2);
    f.addEntity(b3);
    f.addEntity(b4);
    b1->connectInput(0, s1, 0);
    b2->connectInput(0, s2, 0);
    b3->connectInput(0, s3, 0);
    b4->connectInput(0, s4, 0);
    m->connectInput(0, b1, 0);
    m->connectInput(1, b2, 0);
    m->connectInput(2, b3, 0);
    m->connectInput(3, b4, 0);
    m->setRecipe(r);
    f.advanceBy(12 * 1000 + 300, [&]() {
        EXPECT_EQ(m->getOutputStack(0)->getAmount(), 3);
        EXPECT_EQ(m->getOutputStack(1)->getAmount(), 3);
        EXPECT_EQ(m->getOutputStack(0)->getResource(), Resource::SteelIngot);
        EXPECT_EQ(m->getOutputStack(1)->getResource(), Resource::Concrete);
    });
}