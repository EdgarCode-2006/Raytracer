#include <gtest/gtest.h>
#include "utils/Material.hpp"

using Graphic::Pixel;
using Graphic::Material;

TEST(Pixel, DefaultConstructionIsBlackOpaque)
{
    Pixel p;
    EXPECT_EQ(p._r, 0);
    EXPECT_EQ(p._g, 0);
    EXPECT_EQ(p._b, 0);
    EXPECT_EQ(p._a, 255);
}

TEST(Pixel, ThreeArgCtorSetsFullAlpha)
{
    Pixel p(100, 150, 200);
    EXPECT_EQ(p._r, 100);
    EXPECT_EQ(p._g, 150);
    EXPECT_EQ(p._b, 200);
    EXPECT_EQ(p._a, 255);
}

TEST(Pixel, FourArgCtorSetsAllChannels)
{
    Pixel p(10, 20, 30, 128);
    EXPECT_EQ(p._r, 10);
    EXPECT_EQ(p._g, 20);
    EXPECT_EQ(p._b, 30);
    EXPECT_EQ(p._a, 128);
}

TEST(Pixel, MaxValues)
{
    Pixel p(255, 255, 255, 255);
    EXPECT_EQ(p._r, 255);
    EXPECT_EQ(p._g, 255);
    EXPECT_EQ(p._b, 255);
    EXPECT_EQ(p._a, 255);
}

TEST(Material, DefaultIsWhiteWithNoReflection)
{
    Material m;
    EXPECT_EQ(m.color._r, 255);
    EXPECT_EQ(m.color._g, 255);
    EXPECT_EQ(m.color._b, 255);
    EXPECT_EQ(m.color._a, 255);
    EXPECT_FLOAT_EQ(m.reflectivity, 0.0f);
    EXPECT_FLOAT_EQ(m.refractivity, 0.0f);
    EXPECT_FLOAT_EQ(m.ior, 1.0f);
}

TEST(Material, ParameterizedConstructionStoresValues)
{
    Pixel red(255, 0, 0, 255);
    Material m(red, 0.5f, 0.2f, 1.5f, 0.0f);
    EXPECT_EQ(m.color._r, 255);
    EXPECT_EQ(m.color._g, 0);
    EXPECT_EQ(m.color._b, 0);
    EXPECT_FLOAT_EQ(m.reflectivity, 0.5f);
    EXPECT_FLOAT_EQ(m.refractivity, 0.2f);
    EXPECT_FLOAT_EQ(m.ior, 1.5f);
}

TEST(Material, FullyReflective)
{
    Material m(Pixel(255, 255, 255), 1.0f, 0.0f, 1.0f, 0.0f);
    EXPECT_FLOAT_EQ(m.reflectivity, 1.0f);
    EXPECT_FLOAT_EQ(m.refractivity, 0.0f);
}

TEST(Material, FullyRefractive)
{
    Material m(Pixel(200, 200, 200), 0.0f, 1.0f, 1.33f, 0.0f);
    EXPECT_FLOAT_EQ(m.refractivity, 1.0f);
    EXPECT_FLOAT_EQ(m.ior, 1.33f);
}