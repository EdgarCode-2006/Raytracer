#ifndef WORLD_HPP_
#define WORLD_HPP_

#include "opti/BVH/Instance.hpp"
#include "opti/BVH/TLAS.hpp"
#include "opti/BVH/TLASHit.hpp"
#include <vector>

namespace RayTracer {
    class World {
        public:
            World() = default;
            ~World() = default;

            World(const World &) = delete;
            World &operator=(const World &) = delete;
            World(World &&) = default;
            World &operator=(World &&) = default;

            void reserve(uint32_t instanceCount) {_instances.reserve(instanceCount);}

            Instance &addInstance(const GeometricData &geo, uint32_t meshId, const Math::Matrix4 &transform = Math::Matrix4{}, const Graphic::Material &mat = Graphic::Material{});
            void buildTLAS() {_tlas.build(std::span<const Instance>(_instances));}

            Instance &getInstance(uint32_t index) noexcept {return _instances[index];}
            const Instance &getInstance(uint32_t index) const noexcept {return _instances[index];}
            uint32_t getInstanceCount() const noexcept {return static_cast<uint32_t>(_instances.size());}

            const TLAS &getTLAS() const noexcept {return _tlas;}
            TLASHit intersect(const Ray &ray) const noexcept {return _tlas.traverse(ray);}

        private:
            std::vector<Instance> _instances;
            TLAS _tlas;
    };
}


#endif