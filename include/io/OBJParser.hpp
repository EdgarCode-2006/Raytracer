#ifndef OBJPARSER_HPP_
#define OBJPARSER_HPP_

#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include <string>
#include <vector>
#include "utils/UV.hpp"

namespace OBJParser {
    struct IndexGroup { int v, t, n; };

    class OBJParser
    {
        public:
            std::vector<RayTracer::Triangle> load(const std::string &path);

        private:
            std::vector<Math::Vector3D> _vertices;
            std::vector<Math::Vector3D> _normals;
            std::vector<Graphic::UV> _uvs;
    };
}
#endif
