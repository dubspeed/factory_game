#include "gtest/gtest.h"
#include "../src/factory.h"
using namespace Fac;

TEST(ReconnectLinks, SingleMachineBelt) {
    auto w = GameWorld();
    const auto m1 = std::make_shared<SingleMachine>(SingleMachine());
    const auto m2 = std::make_shared<SingleMachine>(SingleMachine());
    const auto belt = std::make_shared<Belt>(Belt(1));
    belt->connectInput(0, m1, 0);
    m2->connectInput(0, belt, 0);
    m1->setRecipe(recipe_IronIngot);
    m2->setRecipe(recipe_IronPlate);
    m1->getOutputStack(0)->addAmount(10, Resource::IronIngot);
    belt->getOutputStack(0)->addAmount(33, Resource::IronIngot);
    w.addEntity(m1);
    w.addEntity(m2);
    w.addEntity(belt);

    json j = w;

    auto x = GameWorld();
    x = j.get<GameWorld>();

    EXPECT_EQ(x.getEntities().size(), 3);
    EXPECT_EQ(x.getEntities()[0]->getId(), m1->getId());
    EXPECT_EQ(x.getEntities()[1]->getId(), m2->getId());
    EXPECT_EQ(x.getEntities()[2]->getId(), belt->getId());
    EXPECT_EQ(std::dynamic_pointer_cast<OutputStackProvider>(x.getEntities()[0])->getOutputStack(0)->getAmount(), 10);
    EXPECT_EQ(std::dynamic_pointer_cast<OutputStackProvider>(x.getEntities()[2])->getOutputStack(0)->getAmount(), 33);
    EXPECT_EQ(std::dynamic_pointer_cast<InputStackProvider>(x.getEntities()[1])->getInputStack(0)->getAmount(), 33);
}

TEST(ReconnectLinks, ResourceNodeResourceExtractorBelt) {
    auto w = GameWorld();
    const auto n = std::make_shared<ResourceNode>(ResourceNode());
    const auto e = std::make_shared<ResourceExtractor>(ResourceExtractor());
    const auto belt = std::make_shared<Belt>(Belt(1));
    e->setResourceNode(n);
    belt->connectInput(0, e, 0);
    e->getOutputStack(0)->addAmount(10, Resource::IronOre);
    w.addEntity(n);
    w.addEntity(e);
    w.addEntity(belt);

    json j = w;
    std::cout << j.dump(4) << std::endl;

    auto x = GameWorld();
    x = j.get<GameWorld>();

    EXPECT_EQ(x.getEntities().size(), 3);
    EXPECT_EQ(x.getEntities()[0]->getId(), n->getId());
    EXPECT_EQ(x.getEntities()[1]->getId(), e->getId());
    EXPECT_EQ(x.getEntities()[2]->getId(), belt->getId());
    EXPECT_EQ(std::dynamic_pointer_cast<ResourceExtractor>(x.getEntities()[1])->getResourceNode()->getId(), n->getId());
    EXPECT_EQ(std::dynamic_pointer_cast<OutputStackProvider>(x.getEntities()[1])->getOutputStack(0)->getAmount(), 10);
    EXPECT_EQ(std::dynamic_pointer_cast<InputStackProvider>(x.getEntities()[2])->getInputStack(0)->getAmount(), 10);
}

TEST(ReconnectLinks, StorageSplitterMachines) {
    auto w = GameWorld();
    const auto s = std::make_shared<Storage>(Storage());
    const auto belt1 = std::make_shared<Belt>(Belt(1));
    const auto sp = std::make_shared<Splitter>(Splitter());
    const auto belt2 = std::make_shared<Belt>(Belt(1));
    const auto m1 = std::make_shared<SingleMachine>(SingleMachine());
    s->setMaxItemStacks(1);

    s->getInputStack(0)->addAmount(33, Resource::IronOre);
    sp->getOutputStack(0)->addAmount(3, Resource::IronOre);
    belt1->connectInput(0, s, 0);
    sp->connectInput(0, belt1, 0);
    belt2->connectInput(0, sp, 0);
    m1->connectInput(0, belt2, 0);
    m1->setRecipe(recipe_IronIngot);
    w.addEntity(s);
    w.addEntity(belt1);
    w.addEntity(sp);
    w.addEntity(belt2);
    w.addEntity(m1);

    json j = w;
    //std::cout << j.dump(4) << std::endl;

    auto x = GameWorld();
    x = j.get<GameWorld>();

    EXPECT_EQ(x.getEntities().size(), 5);
    EXPECT_EQ(x.getEntities()[0]->getId(), s->getId());
    EXPECT_EQ(x.getEntities()[1]->getId(), belt1->getId());
    EXPECT_EQ(x.getEntities()[2]->getId(), sp->getId());
    EXPECT_EQ(x.getEntities()[3]->getId(), belt2->getId());
    EXPECT_EQ(x.getEntities()[4]->getId(), m1->getId());
    EXPECT_EQ(std::dynamic_pointer_cast<Storage>(x.getEntities()[0])->getAmount(Resource::IronOre), 33);
    EXPECT_EQ(std::dynamic_pointer_cast<InputStackProvider>(x.getEntities()[1])->getInputStack(0)->getAmount(), 33);
    EXPECT_EQ(std::dynamic_pointer_cast<OutputStackProvider>(x.getEntities()[2])->getOutputStack(0)->getAmount(), 3);
    EXPECT_EQ(std::dynamic_pointer_cast<InputStackProvider>(x.getEntities()[3])->getInputStack(0)->getAmount(), 3);
    EXPECT_EQ(std::dynamic_pointer_cast<InputStackProvider>(x.getEntities()[4])->getInputStack(0)->getAmount(), 3);
}

TEST(ReconnectLinks, FeedStorage) {
    auto w = GameWorld();
    const auto s = std::make_shared<Storage>(Storage());
    const auto belt = std::make_shared<Belt>(Belt(1));
    s->connectInput(0, belt, 0);
    s->setMaxItemStacks(1);
    belt->getOutputStack(0)->addAmount(33, Resource::IronOre);
    w.addEntity(s);
    w.addEntity(belt);

    json j = w;
    //std::cout << j.dump(4) << std::endl;

    auto x = GameWorld();
    x = j.get<GameWorld>();

    EXPECT_EQ(x.getEntities().size(), 2);
    EXPECT_EQ(x.getEntities()[0]->getId(), s->getId());
    EXPECT_EQ(x.getEntities()[1]->getId(), belt->getId());
    EXPECT_EQ(std::dynamic_pointer_cast<Storage>(x.getEntities()[0])->getInputStack(0)->getAmount(), 33);
}