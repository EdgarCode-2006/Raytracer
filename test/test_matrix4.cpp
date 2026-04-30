#include <gtest/gtest.h>
#include "utils/Matrix4.hpp"

using RayTracer::Matrix4;
using RayTracer::AABB;
using Math::Vector3D;

static constexpr double EPS = 1e-9;

TEST(Matrix4, DefaultIsIdentity)
{
    Matrix4 id;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            EXPECT_DOUBLE_EQ(id.m[i][j], i == j ? 1.0 : 0.0);
}

TEST(Matrix4, IdentityTransformPoint)
{
    Matrix4 id;
    Vector3D p(3.0, 4.0, 5.0);
    Vector3D r = id.transformPoint(p);
    EXPECT_NEAR(r.x(), 3.0, EPS);
    EXPECT_NEAR(r.y(), 4.0, EPS);
    EXPECT_NEAR(r.z(), 5.0, EPS);
}

TEST(Matrix4, IdentityTransformDirection)
{
    Matrix4 id;
    Vector3D d(1.0, 0.0, 0.0);
    Vector3D r = id.transformDirection(d);
    EXPECT_NEAR(r.x(), 1.0, EPS);
    EXPECT_NEAR(r.y(), 0.0, EPS);
    EXPECT_NEAR(r.z(), 0.0, EPS);
}

TEST(Matrix4, TranslationMovesPoint)
{
    Matrix4 id;
    Matrix4 t = id.translation(5.0, 6.0, 7.0);
    Vector3D p(1.0, 2.0, 3.0);
    Vector3D r = t.transformPoint(p);
    EXPECT_NEAR(r.x(), 6.0, EPS);
    EXPECT_NEAR(r.y(), 8.0, EPS);
    EXPECT_NEAR(r.z(), 10.0, EPS);
}

TEST(Matrix4, TranslationDoesNotAffectDirection)
{
    Matrix4 id;
    Matrix4 t = id.translation(10.0, 20.0, 30.0);
    Vector3D d(1.0, 2.0, 3.0);
    Vector3D r = t.transformDirection(d);
    EXPECT_NEAR(r.x(), 1.0, EPS);
    EXPECT_NEAR(r.y(), 2.0, EPS);
    EXPECT_NEAR(r.z(), 3.0, EPS);
}

TEST(Matrix4, ScaleMultipliesComponents)
{
    Matrix4 id;
    Matrix4 s = id.scale(2.0, 3.0, 4.0);
    Vector3D p(1.0, 1.0, 1.0);
    Vector3D r = s.transformPoint(p);
    EXPECT_NEAR(r.x(), 2.0, EPS);
    EXPECT_NEAR(r.y(), 3.0, EPS);
    EXPECT_NEAR(r.z(), 4.0, EPS);
}

TEST(Matrix4, ScaleUniform)
{
    Matrix4 id;
    Matrix4 s = id.scale(5.0, 5.0, 5.0);
    Vector3D p(2.0, 3.0, 4.0);
    Vector3D r = s.transformPoint(p);
    EXPECT_NEAR(r.x(), 10.0, EPS);
    EXPECT_NEAR(r.y(), 15.0, EPS);
    EXPECT_NEAR(r.z(), 20.0, EPS);
}

TEST(Matrix4, RotationXHalfPi)
{
    Matrix4 id;
    Matrix4 r = id.rotationX(M_PI / 2.0);
    Vector3D y(0.0, 1.0, 0.0);
    Vector3D result = r.transformDirection(y);
    EXPECT_NEAR(result.x(), 0.0, 1e-9);
    EXPECT_NEAR(result.y(), 0.0, 1e-9);
    EXPECT_NEAR(result.z(), 1.0, 1e-9);
}

TEST(Matrix4, RotationYHalfPi)
{
    Matrix4 id;
    Matrix4 r = id.rotationY(M_PI / 2.0);
    Vector3D x(1.0, 0.0, 0.0);
    Vector3D result = r.transformDirection(x);
    EXPECT_NEAR(result.x(), 0.0, 1e-9);
    EXPECT_NEAR(result.y(), 0.0, 1e-9);
    EXPECT_NEAR(result.z(), -1.0, 1e-9);
}

TEST(Matrix4, RotationZHalfPi)
{
    Matrix4 id;
    Matrix4 r = id.rotationZ(M_PI / 2.0);
    Vector3D x(1.0, 0.0, 0.0);
    Vector3D result = r.transformDirection(x);
    EXPECT_NEAR(result.x(), 0.0, 1e-9);
    EXPECT_NEAR(result.y(), 1.0, 1e-9);
    EXPECT_NEAR(result.z(), 0.0, 1e-9);
}

TEST(Matrix4, RotationXFullCircleIsIdentity)
{
    Matrix4 id;
    Matrix4 r = id.rotationX(2.0 * M_PI);
    Vector3D p(1.0, 2.0, 3.0);
    Vector3D result = r.transformDirection(p);
    EXPECT_NEAR(result.x(), 1.0, 1e-9);
    EXPECT_NEAR(result.y(), 2.0, 1e-9);
    EXPECT_NEAR(result.z(), 3.0, 1e-9);
}

TEST(Matrix4, MultiplyByIdentityIsUnchanged)
{
    Matrix4 id;
    Matrix4 t = id.translation(1.0, 2.0, 3.0);
    Matrix4 result = id * t;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            EXPECT_NEAR(result.m[i][j], t.m[i][j], EPS);
}

TEST(Matrix4, MultiplyTwoTranslations)
{
    Matrix4 id;
    Matrix4 t1 = id.translation(1.0, 0.0, 0.0);
    Matrix4 t2 = id.translation(2.0, 0.0, 0.0);
    Matrix4 combined = t1 * t2;
    Vector3D p(0.0, 0.0, 0.0);
    Vector3D r = combined.transformPoint(p);
    EXPECT_NEAR(r.x(), 3.0, EPS);
    EXPECT_NEAR(r.y(), 0.0, EPS);
    EXPECT_NEAR(r.z(), 0.0, EPS);
}

TEST(Matrix4, InverseOfIdentityIsIdentity)
{
    Matrix4 id;
    Matrix4 inv = id.inverse();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            EXPECT_NEAR(inv.m[i][j], i == j ? 1.0 : 0.0, EPS);
}

TEST(Matrix4, InverseOfTranslationUndoesMovePoint)
{
    Matrix4 id;
    Matrix4 t = id.translation(5.0, 0.0, 0.0);
    Matrix4 inv = t.inverse();
    Vector3D p(5.0, 0.0, 0.0);
    Vector3D r = inv.transformPoint(p);
    EXPECT_NEAR(r.x(), 0.0, EPS);
    EXPECT_NEAR(r.y(), 0.0, EPS);
    EXPECT_NEAR(r.z(), 0.0, EPS);
}

TEST(Matrix4, MultiplyByInverseIsIdentity)
{
    Matrix4 id;
    Matrix4 t = id.translation(3.0, -1.0, 2.0);
    Matrix4 inv = t.inverse();
    Matrix4 result = t * inv;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            EXPECT_NEAR(result.m[i][j], i == j ? 1.0 : 0.0, 1e-9);
}

TEST(Matrix4, TransformAABBByTranslation)
{
    AABB box;
    box.expand(Vector3D(0.0, 0.0, 0.0));
    box.expand(Vector3D(1.0, 1.0, 1.0));

    Matrix4 id;
    Matrix4 t = id.translation(2.0, 3.0, 4.0);
    AABB transformed = t.transformAABB(box);

    EXPECT_NEAR(transformed.min().x(), 2.0, EPS);
    EXPECT_NEAR(transformed.min().y(), 3.0, EPS);
    EXPECT_NEAR(transformed.min().z(), 4.0, EPS);
    EXPECT_NEAR(transformed.max().x(), 3.0, EPS);
    EXPECT_NEAR(transformed.max().y(), 4.0, EPS);
    EXPECT_NEAR(transformed.max().z(), 5.0, EPS);
}

TEST(Matrix4, TransformAABBByScale)
{
    AABB box;
    box.expand(Vector3D(1.0, 1.0, 1.0));
    box.expand(Vector3D(2.0, 2.0, 2.0));

    Matrix4 id;
    Matrix4 s = id.scale(2.0, 2.0, 2.0);
    AABB transformed = s.transformAABB(box);

    EXPECT_NEAR(transformed.min().x(), 2.0, EPS);
    EXPECT_NEAR(transformed.max().x(), 4.0, EPS);
}
