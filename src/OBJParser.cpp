#include "../include/OBJParser.hpp"
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
        this->_vertices.push_back(Math::Vector3D(x, y, z));
    }

    void OBJParser::parseNormal(std::istringstream& ss)
    {
        double x, y, z;
        ss >> x >> y >> z;
        this->_normals.push_back(Math::Vector3D(x, y, z));
    }

    void OBJParser::parseFace(std::istringstream& ss, std::vector<RayTracer::Triangle>& out)
    {
        std::vector<int> indices;
        std::string token;

        while (ss >> token)
            indices.push_back(parseIndex(token) - 1);

        for (size_t i = 1; i + 1 < indices.size(); i++)
            out.push_back({ this->_vertices[indices[0]], this->_vertices[indices[i]], this->_vertices[indices[i + 1]] });
    }

    std::vector<RayTracer::Triangle> OBJParser::load(const std::string& path)
    {
        std::ifstream file(path);
        std::vector<RayTracer::Triangle> triangles;

        if (!file.is_open())
            return triangles;
        this->_vertices.clear();
        this->_normals.clear();

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
