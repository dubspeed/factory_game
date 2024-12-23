#include "gtest/gtest.h"
#include "../src/factory.h"
using namespace Fac;

TEST(ReconnectLinks, SingleMachineBelt) {
    auto w = Factory();
    const auto m1 = std::make_shared<Machine>(Machine());
    const auto m2 = std::make_shared<Machine>(Machine());
    const auto belt = std::make_shared<Belt>(Belt(1));
    belt->connectInput(0, m1, 0);
    m2->connectInput(0, belt, 0);
    m1->setRecipe(recipe_IronIngot);
    m2->setRecipe(recipe_IronPlate);
    m1->getOutputStack(0)->addAmount(10, Resource::IronIngot);
    belt->getOutputStack(0)->addAmount(1, Resource::IronIngot);
    w.addEntity(m1);
    w.addEntity(m2);
    w.addEntity(belt);

    json j = w;

    auto x = Factory();
    x = j.get<Factory>();

    EXPECT_EQ(x.getEntities().size(), 3);
    EXPECT_EQ(x.getEntities()[0]->getId(), m1->getId());
    EXPECT_EQ(x.getEntities()[1]->getId(), m2->getId());
    EXPECT_EQ(x.getEntities()[2]->getId(), belt->getId());
    EXPECT_EQ(std::dynamic_pointer_cast<OutputStackProvider>(x.getEntities()[0])->getOutputStack(0)->getAmount(), 10);
    EXPECT_EQ(std::dynamic_pointer_cast<OutputStackProvider>(x.getEntities()[2])->getOutputStack(0)->getAmount(), 1);
    EXPECT_EQ(std::dynamic_pointer_cast<InputStackProvider>(x.getEntities()[1])->getInputStack(0)->getAmount(), 1);
}

TEST(ReconnectLinks, ResourceNodeResourceExtractorBelt) {
    auto w = Factory();
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
    //std::cout << j.dump(4) << std::endl;

    auto x = Factory();
    x = j.get<Factory>();

    EXPECT_EQ(x.getEntities().size(), 3);
    EXPECT_EQ(x.getEntities()[0]->getId(), n->getId());
    EXPECT_EQ(x.getEntities()[1]->getId(), e->getId());
    EXPECT_EQ(x.getEntities()[2]->getId(), belt->getId());
    EXPECT_EQ(std::dynamic_pointer_cast<ResourceExtractor>(x.getEntities()[1])->getResourceNode()->getId(), n->getId());
    EXPECT_EQ(std::dynamic_pointer_cast<OutputStackProvider>(x.getEntities()[1])->getOutputStack(0)->getAmount(), 10);
    EXPECT_EQ(std::dynamic_pointer_cast<InputStackProvider>(x.getEntities()[2])->getInputStack(0)->getAmount(), 10);
}

TEST(ReconnectLinks, StorageSplitterMachines) {
    auto w = Factory();
    const auto s = std::make_shared<Storage>(Storage());
    const auto belt1 = std::make_shared<Belt>(Belt(1));
    const auto sp = std::make_shared<Splitter>(Splitter());
    const auto belt2 = std::make_shared<Belt>(Belt(1));
    const auto m1 = std::make_shared<Machine>(Machine());
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

    w.advanceBy(9000, [](){});

    json j = w;
    //std::cout << j.dump(4) << std::endl;

    auto x = Factory();
    x = j.get<Factory>();

    EXPECT_EQ(x.getEntities().size(), 5);
    EXPECT_EQ(x.getEntities()[0]->getId(), s->getId());
    EXPECT_EQ(x.getEntities()[1]->getId(), belt1->getId());
    EXPECT_EQ(x.getEntities()[2]->getId(), sp->getId());
    EXPECT_EQ(x.getEntities()[3]->getId(), belt2->getId());
    EXPECT_EQ(x.getEntities()[4]->getId(), m1->getId());
    EXPECT_EQ(std::dynamic_pointer_cast<Storage>(x.getEntities()[0])->getAmount(Resource::IronOre), 33);
    EXPECT_EQ(std::dynamic_pointer_cast<InputStackProvider>(x.getEntities()[1])->getInputStack(0)->getAmount(), 0);
    EXPECT_EQ(std::dynamic_pointer_cast<OutputStackProvider>(x.getEntities()[2])->getOutputStack(0)->getAmount(), 0);
    EXPECT_EQ(std::dynamic_pointer_cast<InputStackProvider>(x.getEntities()[3])->getInputStack(0)->getAmount(), 0);
    EXPECT_EQ(std::dynamic_pointer_cast<InputStackProvider>(x.getEntities()[4])->getInputStack(1)->getAmount(), 0);
    EXPECT_EQ(std::dynamic_pointer_cast<Machine>(x.getEntities()[4])->getOutputStack(0)->getAmount(), 0);
}

TEST(ReconnectLinks, FeedStorage) {
    auto w = Factory();
    const auto s1 = std::make_shared<Storage>(Storage());
    const auto s2 = std::make_shared<Storage>(Storage());
    const auto belt = std::make_shared<Belt>(Belt(1));
    s1->setMaxItemStacks(1);
    s1->getOutputStack(0)->addAmount(33, Resource::IronOre);
    belt->connectInput(0, s1, 0);
    s2->connectInput(0, belt, 0);
    s2->setMaxItemStacks(1);
    w.addEntity(s1);
    w.addEntity(s2);
    w.addEntity(belt);

    w.advanceBy(30000, [](){});

    json j = w;
    //std::cout << j.dump(4) << std::endl;

    auto x = Factory();
    x = j.get<Factory>();

    EXPECT_EQ(x.getEntities().size(), 3);
    EXPECT_EQ(x.getEntities()[0]->getId(), s1->getId());
    EXPECT_EQ(x.getEntities()[1]->getId(), s2->getId());
    EXPECT_EQ(x.getEntities()[2]->getId(), belt->getId());
    auto result = std::dynamic_pointer_cast<Storage>(x.getEntities()[1]);
    EXPECT_EQ(result->getAmount(Resource::IronOre), 29);
}