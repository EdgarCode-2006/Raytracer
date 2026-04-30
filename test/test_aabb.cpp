#include <gtest/gtest.h>
#include "opti/BVH/AABB.hpp"

using RayTracer::AABB;
using Math::Vector3D;

static constexpr double EPS = 1e-9;

TEST(AABB, DefaultConstructionIsInvalid)
{
    AABB box;
    EXPECT_FALSE(box.isValid());
}

TEST(AABB, ExpandSinglePointBecomesValid)
{
    AABB box;
    box.expand(Vector3D(1.0, 2.0, 3.0));
    EXPECT_TRUE(box.isValid());
}

TEST(AABB, ExpandSinglePointBoundsMatch)
{
    AABB box;
    box.expand(Vector3D(1.0, 2.0, 3.0));
    EXPECT_DOUBLE_EQ(box.min().x(), 1.0);
    EXPECT_DOUBLE_EQ(box.min().y(), 2.0);
    EXPECT_DOUBLE_EQ(box.min().z(), 3.0);
    EXPECT_DOUBLE_EQ(box.max().x(), 1.0);
    EXPECT_DOUBLE_EQ(box.max().y(), 2.0);
    EXPECT_DOUBLE_EQ(box.max().z(), 3.0);
}

TEST(AABB, ExpandTwoPointsCorrectMinMax)
{
    AABB box;
    box.expand(Vector3D(0.0, 0.0, 0.0));
    box.expand(Vector3D(2.0, 3.0, 4.0));
    EXPECT_DOUBLE_EQ(box.min().x(), 0.0);
    EXPECT_DOUBLE_EQ(box.min().y(), 0.0);
    EXPECT_DOUBLE_EQ(box.min().z(), 0.0);
    EXPECT_DOUBLE_EQ(box.max().x(), 2.0);
    EXPECT_DOUBLE_EQ(box.max().y(), 3.0);
    EXPECT_DOUBLE_EQ(box.max().z(), 4.0);
}

TEST(AABB, ExpandNegativePoints)
{
    AABB box;
    box.expand(Vector3D(-1.0, -2.0, -3.0));
    box.expand(Vector3D(1.0, 2.0, 3.0));
    EXPECT_DOUBLE_EQ(box.min().x(), -1.0);
    EXPECT_DOUBLE_EQ(box.min().y(), -2.0);
    EXPECT_DOUBLE_EQ(box.min().z(), -3.0);
    EXPECT_DOUBLE_EQ(box.max().x(), 1.0);
    EXPECT_DOUBLE_EQ(box.max().y(), 2.0);
    EXPECT_DOUBLE_EQ(box.max().z(), 3.0);
}

TEST(AABB, ExpandWithOtherAABB)
{
    AABB a, b;
    a.expand(Vector3D(0.0, 0.0, 0.0));
    a.expand(Vector3D(1.0, 1.0, 1.0));
    b.expand(Vector3D(2.0, 2.0, 2.0));
    b.expand(Vector3D(3.0, 3.0, 3.0));
    a.expand(b);
    EXPECT_DOUBLE_EQ(a.min().x(), 0.0);
    EXPECT_DOUBLE_EQ(a.max().x(), 3.0);
}

TEST(AABB, CenterOfUnitCube)
{
    AABB box;
    box.expand(Vector3D(0.0, 0.0, 0.0));
    box.expand(Vector3D(2.0, 2.0, 2.0));
    Vector3D c = box.center();
    EXPECT_NEAR(c.x(), 1.0, EPS);
    EXPECT_NEAR(c.y(), 1.0, EPS);
    EXPECT_NEAR(c.z(), 1.0, EPS);
}

TEST(AABB, CenterGeneral)
{
    AABB box;
    box.expand(Vector3D(1.0, 2.0, 3.0));
    box.expand(Vector3D(3.0, 4.0, 5.0));
    Vector3D c = box.center();
    EXPECT_NEAR(c.x(), 2.0, EPS);
    EXPECT_NEAR(c.y(), 3.0, EPS);
    EXPECT_NEAR(c.z(), 4.0, EPS);
}

TEST(AABB, SizeOfBox)
{
    AABB box;
    box.expand(Vector3D(0.0, 0.0, 0.0));
    box.expand(Vector3D(2.0, 3.0, 4.0));
    Vector3D s = box.size();
    EXPECT_NEAR(s.x(), 2.0, EPS);
    EXPECT_NEAR(s.y(), 3.0, EPS);
    EXPECT_NEAR(s.z(), 4.0, EPS);
}

TEST(AABB, SurfaceAreaCube)
{
    AABB box;
    box.expand(Vector3D(0.0, 0.0, 0.0));
    box.expand(Vector3D(1.0, 1.0, 1.0));
    EXPECT_NEAR(box.surfaceArea(), 6.0, EPS);
}

TEST(AABB, SurfaceAreaRectangularBox)
{
    AABB box;
    box.expand(Vector3D(0.0, 0.0, 0.0));
    box.expand(Vector3D(2.0, 3.0, 4.0));
    EXPECT_NEAR(box.surfaceArea(), 52.0, EPS);
}

TEST(AABB, SurfaceAreaInvalidReturnsZero)
{
    AABB box;
    EXPECT_DOUBLE_EQ(box.surfaceArea(), 0.0);
}

TEST(AABB, IsValidAfterMultipleExpands)
{
    AABB box;
    for (int i = 0; i < 5; i++)
        box.expand(Vector3D(i, i, i));
    EXPECT_TRUE(box.isValid());
}
