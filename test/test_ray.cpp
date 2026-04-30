#include <gtest/gtest.h>
#include "utils/rayTracer.hpp"

using RayTracer::Ray;
using Math::Vector3D;

TEST(Ray, ConstructionStoresOriginAndDirection)
{
    Ray r({1.0, 2.0, 3.0}, {0.0, 1.0, 0.0});
    EXPECT_DOUBLE_EQ(r.origin.x(), 1.0);
    EXPECT_DOUBLE_EQ(r.origin.y(), 2.0);
    EXPECT_DOUBLE_EQ(r.origin.z(), 3.0);
    EXPECT_DOUBLE_EQ(r.direction.x(), 0.0);
    EXPECT_DOUBLE_EQ(r.direction.y(), 1.0);
    EXPECT_DOUBLE_EQ(r.direction.z(), 0.0);
}

TEST(Ray, MutableFields)
{
    Ray r({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
    r.origin.x() = 5.0;
    r.direction = {0.0, 0.0, 1.0};
    EXPECT_DOUBLE_EQ(r.origin.x(), 5.0);
    EXPECT_DOUBLE_EQ(r.direction.z(), 1.0);
}

TEST(Ray, PointAlongRay)
{
    Ray r({1.0, 0.0, 0.0}, {0.0, 1.0, 0.0});
    double t = 3.0;
    Vector3D p = r.origin + r.direction * t;
    EXPECT_DOUBLE_EQ(p.x(), 1.0);
    EXPECT_DOUBLE_EQ(p.y(), 3.0);
    EXPECT_DOUBLE_EQ(p.z(), 0.0);
}
