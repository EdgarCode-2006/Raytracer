#ifndef OBJPARSER_HPP_
#define OBJPARSER_HPP_

#include "math.hpp"
#include "rayTracer.hpp"
#include <iostream>
#include <sstream>
#include <vector>

namespace OBJParser {

    class OBJParser
    {
        public:
            std::vector<RayTracer::Triangle> load(const std::string &path);

        private:
            std::vector<Math::Vector3D> _vertices;
            std::vector<Math::Vector3D> _normals;

            void parseVertex(std::istringstream &line);
            void parseNormal(std::istringstream &line);
            void parseFace(std::istringstream &line, std::vector<RayTracer::Triangle>& out);
            int parseIndex(const std::string& token);
    };
}

#endif