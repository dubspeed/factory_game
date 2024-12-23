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