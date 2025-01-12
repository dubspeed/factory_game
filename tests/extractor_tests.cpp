#include <memory>

#include "gtest/gtest.h"
#include "../src/factory.h"

using namespace Fac;

TEST(Extractor, CalculateExtractionRate) {

    auto r = std::make_shared<ResourceNode>(ResourceNode());
    r->setResource(Resource::IronOre);
    r->setResourceQuality(ResourceQuality::Impure);
    auto e = std::make_shared<Extractor>(Extractor());
    e->setDefaultSpeed(60);
    e->setResourceNode(r);

    EXPECT_EQ(e->getOutputRpm(), 60 * 0.5);
}

TEST(Extractor, DoesNotExtractWhenOutputStackIsFull) {
    auto f = Factory();
    auto r = std::make_shared<ResourceNode>(ResourceNode());
    r->setResource(Resource::IronOre);
    r->setResourceQuality(ResourceQuality::Impure);
    auto e = std::make_shared<Extractor>(Extractor());
    e->setDefaultSpeed(60);
    e->setResourceNode(r);

    e->getOutputStack(0)->addAmount(MAX_STACK_SIZE, Resource::IronOre);
    f.addEntity(r);
    f.addEntity(e);

    f.advanceBy(1000, [&]() {
        EXPECT_EQ(e->getOutputStack(0)->getAmount(), MAX_STACK_SIZE);
        EXPECT_FALSE(e->extracting);
        EXPECT_EQ(e->extraction_progress, 0.0);
    });
}

TEST(Extractor, WillNotExtractIfNoResourceIsSet) {
    auto f = Factory();
    auto e = std::make_shared<Extractor>(Extractor());
    e->setDefaultSpeed(60);
    f.addEntity(e);

    f.advanceBy(1000, [&]() {
        EXPECT_FALSE(e->extracting);
        EXPECT_EQ(e->extraction_progress, 0.0);
    });
}

TEST(Extractor, ReturnsADummyNoneResourceNodeIfResourceNodeNotSet) {
    auto e = Extractor();
    auto r = e.getResourceNode();
    EXPECT_EQ(r->getResource(), Resource::None);
}

TEST(Extractor, ANoneResourceCanNotBeMined) {
    auto f = Factory();
    auto e = std::make_shared<Extractor>(Extractor());
    e->setDefaultSpeed(60);
    f.addEntity(e);

    f.advanceBy(1000 * 10, [&]() {
        EXPECT_FALSE(e->extracting);
        EXPECT_EQ(e->extraction_progress, 0.0);
    });
}

TEST(Extractor, canClearAResourceNode) {
    auto f = Factory();
    auto r = std::make_shared<ResourceNode>(ResourceNode());
    r->setResource(Resource::IronOre);
    r->setResourceQuality(ResourceQuality::Impure);
    auto e = std::make_shared<Extractor>(Extractor());
    e->setResourceNode(r);
    f.addEntity(r);
    f.addEntity(e);

    f.advanceBy(1000 * 10, [&]() {
        EXPECT_TRUE(e->extracting);
        EXPECT_GT(e->extraction_progress, 0.0);
    });
    e->clearResourceNode();
    EXPECT_EQ(e->getResourceNode()->getResource(), Resource::None);
    EXPECT_EQ(e->getOutputStack(0)->getAmount(), 0);
    f.advanceBy(1000 * 10, [&]() {
        EXPECT_FALSE(e->extracting);
        EXPECT_EQ(e->extraction_progress, 0.0);
    });
    EXPECT_EQ(e->getOutputStack(0)->getAmount(), 0);
}

TEST(Extractor, settingANewResourceNodeClearsTheOutput) {
    auto f = Factory();
    auto r = std::make_shared<ResourceNode>(ResourceNode());
    auto r2 = std::make_shared<ResourceNode>(ResourceNode());
    r->setResource(Resource::IronOre);
    r->setResourceQuality(ResourceQuality::Impure);
    r2->setResource(Resource::CopperOre);
    r2->setResourceQuality(ResourceQuality::Normal);
    auto e = std::make_shared<Extractor>(Extractor());
    e->setResourceNode(r);
    f.addEntity(r);
    f.addEntity(e);

    f.advanceBy(1000 * 10, [&]() {
        EXPECT_TRUE(e->extracting);
        EXPECT_GT(e->extraction_progress, 0.0);
    });

    e->setResourceNode(r2);
    EXPECT_EQ(e->getOutputStack(0)->getAmount(), 0);
}