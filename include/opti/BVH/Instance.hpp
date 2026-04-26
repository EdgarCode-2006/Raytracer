#ifndef INSTANCE_HPP_
#define INSTANCE_HPP_

#include "opti/BVH/AABB.hpp"
#include "opti/BVH/BVH.hpp"
#include "opti/BVH/RayHit.hpp"
#include "utils/Matrix4.hpp"
#include "utils/rayTracer.hpp"
#include <memory>
#include <vector>

namespace RayTracer {
    class Instance {
        public:
            Instance(std::shared_ptr<const BVH> blas, std::shared_ptr<const std::vector<Triangle>> triangles, const Matrix4 & transform = Matrix4{});

            void setTransform(const Matrix4 &t) noexcept;

            const AABB &worldAABB() const noexcept;

            RayHit intersect(const Ray &worldRay) const noexcept;

            const BVH &blas() const noexcept;
            const std::vector<Triangle> & triangles() const noexcept;
        
        private:
            std::shared_ptr<const BVH> _blas;
            std::shared_ptr<const std::vector<Triangle>> _triangles;
            Matrix4 _transform;
            Matrix4 _invTransform;
            AABB _worldAABB;
    };
}


#endif