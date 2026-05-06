#include <gtest/gtest.h>
#include "opti/BVH/BVHNode.hpp"

using RayTracer::BVHNode;

TEST(BVHNode, DefaultIsNotLeaf)
{
    BVHNode node;
    EXPECT_FALSE(node.isLeaf());
}

TEST(BVHNode, DefaultLeftOrFirstIsZero)
{
    BVHNode node;
    EXPECT_EQ(node.leftOrFirst(), 0u);
}

TEST(BVHNode, DefaultPrimCountIsZero)
{
    BVHNode node;
    EXPECT_EQ(node.primCount(), 0u);
}

TEST(BVHNode, DefaultBoundsIsInvalid)
{
    BVHNode node;
    EXPECT_FALSE(node.bounds().isValid());
}