#include "io/OBJParser.hpp"
#include <fstream>

namespace OBJParser {

    int OBJParser::parseIndex(const std::string& token)
    {
        return std::stoi(token.substr(0, token.find('/')));
    }

    void OBJParser::parseVertex(std::istringstream& ss)
    {
        double x, y, z;
        ss >> x >> y >> z;
        _vertices.push_back({x, y, z});
    }

    void OBJParser::parseNormal(std::istringstream& ss)
    {
        double x, y, z;
        ss >> x >> y >> z;
        _normals.push_back({x, y, z});
    }

    void OBJParser::parseFace(std::istringstream& ss, std::vector<RayTracer::Triangle>& out)
    {
        std::vector<int> indices;
        std::string token;

        while (ss >> token) {
            int idx = parseIndex(token);
            if (idx < 0)
                idx = static_cast<int>(_vertices.size()) + idx;
            else
                idx -= 1;
            indices.push_back(idx);
        }

        const int vsize = static_cast<int>(_vertices.size());
        for (size_t i = 1; i + 1 < indices.size(); i++) {
            if (indices[0] < 0 || indices[0] >= vsize) continue;
            if (indices[i] < 0 || indices[i] >= vsize) continue;
            if (indices[i + 1] < 0 || indices[i + 1] >= vsize) continue;
            const Math::Vector3D& va = _vertices[indices[0]];
            const Math::Vector3D& vb = _vertices[indices[i]];
            const Math::Vector3D& vc = _vertices[indices[i + 1]];
            RayTracer::Triangle tri(va, vb, vc);
            tri.normal = (vb - va).cross(vc - va).normalize();
            out.push_back(tri);
        }
    }

    std::vector<RayTracer::Triangle> OBJParser::load(const std::string& path)
    {
        std::ifstream file(path);
        std::vector<RayTracer::Triangle> triangles;

        if (!file.is_open())
            return triangles;

        _vertices.clear();
        _normals.clear();

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string type;
            ss >> type;
            if (type == "v")
                parseVertex(ss);
            else if (type == "vn")
                parseNormal(ss);
            else if (type == "f")
                parseFace(ss, triangles);
        }
        return triangles;
    }
}
