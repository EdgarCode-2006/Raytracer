#include <gtest/gtest.h>
#include "utils/math.hpp"

using Math::Vector3D;

static constexpr double EPS = 1e-9;

TEST(Vector3D, DefaultConstructionIsZero)
{
    Vector3D v;
    EXPECT_DOUBLE_EQ(v.x(), 0.0);
    EXPECT_DOUBLE_EQ(v.y(), 0.0);
    EXPECT_DOUBLE_EQ(v.z(), 0.0);
}

TEST(Vector3D, ParameterizedConstruction)
{
    Vector3D v(1.0, 2.0, 3.0);
    EXPECT_DOUBLE_EQ(v.x(), 1.0);
    EXPECT_DOUBLE_EQ(v.y(), 2.0);
    EXPECT_DOUBLE_EQ(v.z(), 3.0);
}

TEST(Vector3D, IndexAccessorGet)
{
    Vector3D v(4.0, 5.0, 6.0);
    EXPECT_DOUBLE_EQ(v[0], 4.0);
    EXPECT_DOUBLE_EQ(v[1], 5.0);
    EXPECT_DOUBLE_EQ(v[2], 6.0);
}

TEST(Vector3D, IndexAccessorSet)
{
    Vector3D v;
    v[0] = 7.0;
    v[1] = 8.0;
    v[2] = 9.0;
    EXPECT_DOUBLE_EQ(v.x(), 7.0);
    EXPECT_DOUBLE_EQ(v.y(), 8.0);
    EXPECT_DOUBLE_EQ(v.z(), 9.0);
}

TEST(Vector3D, MutableXYZSetters)
{
    Vector3D v;
    v.x() = 1.5;
    v.y() = 2.5;
    v.z() = 3.5;
    EXPECT_DOUBLE_EQ(v[0], 1.5);
    EXPECT_DOUBLE_EQ(v[1], 2.5);
    EXPECT_DOUBLE_EQ(v[2], 3.5);
}

TEST(Vector3D, Addition)
{
    Vector3D a(1.0, 2.0, 3.0);
    Vector3D b(4.0, 5.0, 6.0);
    Vector3D r = a + b;
    EXPECT_DOUBLE_EQ(r.x(), 5.0);
    EXPECT_DOUBLE_EQ(r.y(), 7.0);
    EXPECT_DOUBLE_EQ(r.z(), 9.0);
}

TEST(Vector3D, Subtraction)
{
    Vector3D a(5.0, 7.0, 9.0);
    Vector3D b(1.0, 2.0, 3.0);
    Vector3D r = a - b;
    EXPECT_DOUBLE_EQ(r.x(), 4.0);
    EXPECT_DOUBLE_EQ(r.y(), 5.0);
    EXPECT_DOUBLE_EQ(r.z(), 6.0);
}

TEST(Vector3D, ScalarMultiplicationRight)
{
    Vector3D v(1.0, 2.0, 3.0);
    Vector3D r = v * 2.0;
    EXPECT_DOUBLE_EQ(r.x(), 2.0);
    EXPECT_DOUBLE_EQ(r.y(), 4.0);
    EXPECT_DOUBLE_EQ(r.z(), 6.0);
}

TEST(Vector3D, ScalarMultiplicationLeft)
{
    Vector3D v(1.0, 2.0, 3.0);
    Vector3D r = 3.0 * v;
    EXPECT_DOUBLE_EQ(r.x(), 3.0);
    EXPECT_DOUBLE_EQ(r.y(), 6.0);
    EXPECT_DOUBLE_EQ(r.z(), 9.0);
}

TEST(Vector3D, ScalarDivision)
{
    Vector3D v(4.0, 6.0, 8.0);
    Vector3D r = v / 2.0;
    EXPECT_DOUBLE_EQ(r.x(), 2.0);
    EXPECT_DOUBLE_EQ(r.y(), 3.0);
    EXPECT_DOUBLE_EQ(r.z(), 4.0);
}

TEST(Vector3D, AddAssignVector)
{
    Vector3D v(1.0, 2.0, 3.0);
    v += Vector3D(1.0, 1.0, 1.0);
    EXPECT_DOUBLE_EQ(v.x(), 2.0);
    EXPECT_DOUBLE_EQ(v.y(), 3.0);
    EXPECT_DOUBLE_EQ(v.z(), 4.0);
}

TEST(Vector3D, AddAssignScalar)
{
    Vector3D v(1.0, 2.0, 3.0);
    v += 10.0;
    EXPECT_DOUBLE_EQ(v.x(), 11.0);
    EXPECT_DOUBLE_EQ(v.y(), 12.0);
    EXPECT_DOUBLE_EQ(v.z(), 13.0);
}

TEST(Vector3D, SubAssignVector)
{
    Vector3D v(5.0, 5.0, 5.0);
    v -= Vector3D(1.0, 2.0, 3.0);
    EXPECT_DOUBLE_EQ(v.x(), 4.0);
    EXPECT_DOUBLE_EQ(v.y(), 3.0);
    EXPECT_DOUBLE_EQ(v.z(), 2.0);
}

TEST(Vector3D, MulAssignScalar)
{
    Vector3D v(2.0, 3.0, 4.0);
    v *= 2.0;
    EXPECT_DOUBLE_EQ(v.x(), 4.0);
    EXPECT_DOUBLE_EQ(v.y(), 6.0);
    EXPECT_DOUBLE_EQ(v.z(), 8.0);
}

TEST(Vector3D, DivAssignScalar)
{
    Vector3D v(4.0, 6.0, 8.0);
    v /= 2.0;
    EXPECT_DOUBLE_EQ(v.x(), 2.0);
    EXPECT_DOUBLE_EQ(v.y(), 3.0);
    EXPECT_DOUBLE_EQ(v.z(), 4.0);
}

TEST(Vector3D, DotProductOrthogonal)
{
    Vector3D x(1.0, 0.0, 0.0);
    Vector3D y(0.0, 1.0, 0.0);
    EXPECT_DOUBLE_EQ(x.dot(y), 0.0);
}

TEST(Vector3D, DotProductParallel)
{
    Vector3D a(2.0, 0.0, 0.0);
    Vector3D b(3.0, 0.0, 0.0);
    EXPECT_DOUBLE_EQ(a.dot(b), 6.0);
}

TEST(Vector3D, DotProductGeneral)
{
    Vector3D a(1.0, 2.0, 3.0);
    Vector3D b(4.0, 5.0, 6.0);
    EXPECT_DOUBLE_EQ(a.dot(b), 32.0);
}

TEST(Vector3D, CrossProductXY)
{
    Vector3D x(1.0, 0.0, 0.0);
    Vector3D y(0.0, 1.0, 0.0);
    Vector3D z = x.cross(y);
    EXPECT_NEAR(z.x(), 0.0, EPS);
    EXPECT_NEAR(z.y(), 0.0, EPS);
    EXPECT_NEAR(z.z(), 1.0, EPS);
}

TEST(Vector3D, CrossProductYZ)
{
    Vector3D y(0.0, 1.0, 0.0);
    Vector3D z(0.0, 0.0, 1.0);
    Vector3D x = y.cross(z);
    EXPECT_NEAR(x.x(), 1.0, EPS);
    EXPECT_NEAR(x.y(), 0.0, EPS);
    EXPECT_NEAR(x.z(), 0.0, EPS);
}

TEST(Vector3D, CrossProductAntiCommutative)
{
    Vector3D a(1.0, 2.0, 3.0);
    Vector3D b(4.0, 5.0, 6.0);
    Vector3D ab = a.cross(b);
    Vector3D ba = b.cross(a);
    EXPECT_NEAR(ab.x(), -ba.x(), EPS);
    EXPECT_NEAR(ab.y(), -ba.y(), EPS);
    EXPECT_NEAR(ab.z(), -ba.z(), EPS);
}

TEST(Vector3D, LengthUnitX)
{
    Vector3D v(1.0, 0.0, 0.0);
    EXPECT_DOUBLE_EQ(v.length(), 1.0);
}

TEST(Vector3D, LengthGeneral)
{
    Vector3D v(3.0, 4.0, 0.0);
    EXPECT_DOUBLE_EQ(v.length(), 5.0);
}

TEST(Vector3D, Length3D)
{
    Vector3D v(1.0, 2.0, 2.0);
    EXPECT_DOUBLE_EQ(v.length(), 3.0);
}

TEST(Vector3D, NormalizeUnit)
{
    Vector3D v(3.0, 0.0, 0.0);
    Vector3D n = v.normalize();
    EXPECT_NEAR(n.x(), 1.0, EPS);
    EXPECT_NEAR(n.y(), 0.0, EPS);
    EXPECT_NEAR(n.z(), 0.0, EPS);
    EXPECT_NEAR(n.length(), 1.0, EPS);
}

TEST(Vector3D, NormalizeGeneral)
{
    Vector3D v(1.0, 2.0, 2.0);
    Vector3D n = v.normalize();
    EXPECT_NEAR(n.x(), 1.0 / 3.0, EPS);
    EXPECT_NEAR(n.y(), 2.0 / 3.0, EPS);
    EXPECT_NEAR(n.z(), 2.0 / 3.0, EPS);
    EXPECT_NEAR(n.length(), 1.0, EPS);
}

TEST(Vector3D, NormalizeZeroVectorReturnsZero)
{
    Vector3D zero;
    Vector3D n = zero.normalize();
    EXPECT_DOUBLE_EQ(n.x(), 0.0);
    EXPECT_DOUBLE_EQ(n.y(), 0.0);
    EXPECT_DOUBLE_EQ(n.z(), 0.0);
}

TEST(Vector3D, MinFreeFunction)
{
    Vector3D a(1.0, 5.0, 3.0);
    Vector3D b(4.0, 2.0, 6.0);
    Vector3D r = min(a, b);
    EXPECT_DOUBLE_EQ(r.x(), 1.0);
    EXPECT_DOUBLE_EQ(r.y(), 2.0);
    EXPECT_DOUBLE_EQ(r.z(), 3.0);
}

TEST(Vector3D, MaxFreeFunction)
{
    Vector3D a(1.0, 5.0, 3.0);
    Vector3D b(4.0, 2.0, 6.0);
    Vector3D r = max(a, b);
    EXPECT_DOUBLE_EQ(r.x(), 4.0);
    EXPECT_DOUBLE_EQ(r.y(), 5.0);
    EXPECT_DOUBLE_EQ(r.z(), 6.0);
}

TEST(Vector3D, ClampInsideRange)
{
    Vector3D v(2.0, 3.0, 4.0);
    Vector3D lo(1.0, 1.0, 1.0);
    Vector3D hi(5.0, 5.0, 5.0);
    Vector3D r = clamp(v, lo, hi);
    EXPECT_DOUBLE_EQ(r.x(), 2.0);
    EXPECT_DOUBLE_EQ(r.y(), 3.0);
    EXPECT_DOUBLE_EQ(r.z(), 4.0);
}

TEST(Vector3D, ClampBelowMin)
{
    Vector3D v(-1.0, -2.0, -3.0);
    Vector3D lo(0.0, 0.0, 0.0);
    Vector3D hi(1.0, 1.0, 1.0);
    Vector3D r = clamp(v, lo, hi);
    EXPECT_DOUBLE_EQ(r.x(), 0.0);
    EXPECT_DOUBLE_EQ(r.y(), 0.0);
    EXPECT_DOUBLE_EQ(r.z(), 0.0);
}

TEST(Vector3D, ClampAboveMax)
{
    Vector3D v(10.0, 20.0, 30.0);
    Vector3D lo(0.0, 0.0, 0.0);
    Vector3D hi(5.0, 5.0, 5.0);
    Vector3D r = clamp(v, lo, hi);
    EXPECT_DOUBLE_EQ(r.x(), 5.0);
    EXPECT_DOUBLE_EQ(r.y(), 5.0);
    EXPECT_DOUBLE_EQ(r.z(), 5.0);
}
