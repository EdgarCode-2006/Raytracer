#include <gtest/gtest.h>
#include "utils/rayTracer.hpp"

using RayTracer::Triangle;
using Math::Vector3D;

static constexpr double EPS = 1e-9;

TEST(Triangle, VertexAccessors)
{
    Triangle t({1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0});
    EXPECT_DOUBLE_EQ(t.a().x(), 1.0);
    EXPECT_DOUBLE_EQ(t.b().y(), 1.0);
    EXPECT_DOUBLE_EQ(t.c().z(), 1.0);
}

TEST(Triangle, MutableVertexAccessors)
{
    Triangle t({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0});
    t.a().x() = 5.0;
    EXPECT_DOUBLE_EQ(t.a().x(), 5.0);
}

TEST(Triangle, CentroidOriginTriangle)
{
    Triangle t({0.0, 0.0, 0.0}, {3.0, 0.0, 0.0}, {0.0, 3.0, 0.0});
    Vector3D c = t.centroid();
    EXPECT_NEAR(c.x(), 1.0, EPS);
    EXPECT_NEAR(c.y(), 1.0, EPS);
    EXPECT_NEAR(c.z(), 0.0, EPS);
}

TEST(Triangle, CentroidGeneral)
{
    Triangle t({1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0});
    Vector3D c = t.centroid();
    EXPECT_NEAR(c.x(), 4.0, EPS);
    EXPECT_NEAR(c.y(), 5.0, EPS);
    EXPECT_NEAR(c.z(), 6.0, EPS);
}

TEST(Triangle, CentroidSymmetric)
{
    Triangle t({1.0, 0.0, 0.0}, {-0.5, 0.866, 0.0}, {-0.5, -0.866, 0.0});
    Vector3D c = t.centroid();
    EXPECT_NEAR(c.x(), 0.0, 1e-6);
    EXPECT_NEAR(c.y(), 0.0, 1e-6);
    EXPECT_NEAR(c.z(), 0.0, EPS);
}

TEST(Triangle, DirectArrayAccess)
{
    Triangle t({1.0, 0.0, 0.0}, {0.0, 2.0, 0.0}, {0.0, 0.0, 3.0});
    EXPECT_DOUBLE_EQ(t.v[0].x(), 1.0);
    EXPECT_DOUBLE_EQ(t.v[1].y(), 2.0);
    EXPECT_DOUBLE_EQ(t.v[2].z(), 3.0);
}
