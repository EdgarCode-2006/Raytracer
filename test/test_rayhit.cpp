#include <gtest/gtest.h>
#include "opti/BVH/RayHit.hpp"

using RayTracer::RayHit;

TEST(RayHit, DefaultConstructionIsNoHit)
{
    RayHit h;
    EXPECT_FALSE(h.hasHit());
}

TEST(RayHit, DefaultTIsInfinity)
{
    RayHit h;
    EXPECT_GE(h.t(), 1e30);
}

TEST(RayHit, ConstructWithSmallTIsHit)
{
    RayHit h(5.0, 42u, 0.0, 0.0);
    EXPECT_TRUE(h.hasHit());
    EXPECT_DOUBLE_EQ(h.t(), 5.0);
    EXPECT_EQ(h.primitiveIndex(), 42u);
}

TEST(RayHit, ConstructWithExactThresholdIsNoHit)
{
    RayHit h(1e30, 0u, 0.0, 0.0);
    EXPECT_FALSE(h.hasHit());
}

TEST(RayHit, ConstructWithJustBelowThresholdIsHit)
{
    RayHit h(1e29, 0u, 0.0, 0.0);
    EXPECT_TRUE(h.hasHit());
}

TEST(RayHit, ZeroDistanceIsHit)
{
    RayHit h(0.0, 0u, 0.0, 0.0);
    EXPECT_TRUE(h.hasHit());
}

TEST(RayHit, PrimitiveIndexIsPreserved)
{
    RayHit h(1.0, 99999u, 0.0, 0.0);
    EXPECT_EQ(h.primitiveIndex(), 99999u);
}
