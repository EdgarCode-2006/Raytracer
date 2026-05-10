#ifndef OBJPARSER_HPP_
#define OBJPARSER_HPP_

#include "utils/math.hpp"
#include "utils/rayTracer.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
#include "utils/UV.hpp"

namespace OBJParser {
    struct IndexGroup { int v, n; };

    class OBJParser
    {
        public:
            std::vector<RayTracer::Triangle> load(const std::string &path);

        private:
            std::vector<Math::Vector3D> _vertices;
            std::vector<Math::Vector3D> _normals;
            std::vector<Graphic::UV> _uvs;

            void parseVertex(std::istringstream &line);
            void parseNormal(std::istringstream &line);
            void parseUVs(std::istringstream &line);
            void parseFace(std::istringstream &line, std::vector<RayTracer::Triangle>& out);
            std::pair<int, int> parseIndices(const std::string& token);
    };
}
#endif