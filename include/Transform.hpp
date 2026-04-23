#ifndef TRANSFORM_HPP_
#define TRANSFORM_HPP_

#include "utils/math.hpp"
#include "utils/Matrix4x4.hpp"
#include "utils/rayTracer.hpp"

namespace RayTracer {

struct Transform {
    Math::Vector3D position;
    Math::Vector3D rotation;
    Math::Vector3D scale = {1.0, 1.0, 1.0};

    Math::Matrix4x4 getMatrix() const noexcept
    {
        return Math::Matrix4x4::translation(position)
            * Math::Matrix4x4::rotationY(rotation.y())
            * Math::Matrix4x4::rotationX(rotation.x())
            * Math::Matrix4x4::rotationZ(rotation.z())
            * Math::Matrix4x4::scale(scale);
    }

    Triangle apply(const Triangle& tri) const noexcept
    {
        Math::Matrix4x4 mat = getMatrix();

        return Triangle(
            mat.transformPoint(tri.a()),
            mat.transformPoint(tri.b()),
            mat.transformPoint(tri.c())
        );
    }
};

}

#endif
