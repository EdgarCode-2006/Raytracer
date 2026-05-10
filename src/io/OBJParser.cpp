#include "io/OBJParser.hpp"
#include <fstream>

namespace OBJParser {

    // Returns a pair: {vertex_index, normal_index}
    std::pair<int, int> OBJParser::parseIndices(const std::string& token)
    {
        std::vector<std::string> parts;
        std::stringstream ss(token);
        std::string item;
        
        while (std::getline(ss, item, '/')) {
            parts.push_back(item);
        }

        int vIdx = std::stoi(parts[0]);
        int nIdx = -1;

        if (parts.size() == 3 && !parts[2].empty()) {
            nIdx = std::stoi(parts[2]);
        }

        return {vIdx, nIdx};
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

    void OBJParser::parseUVs(std::istringstream& ss)
    {
        float u, v;
        ss >> u >> v;
        _uvs.push_back({u, v});
    }

    void OBJParser::parseFace(std::istringstream& ss, std::vector<RayTracer::Triangle>& out)
    {
        std::vector<IndexGroup> groups;
        std::string token;

        while (ss >> token) {
            auto [vIdx, nIdx] = parseIndices(token);
            
            if (vIdx < 0) vIdx = static_cast<int>(_vertices.size()) + vIdx;
            else vIdx -= 1;

            if (nIdx != -1) {
                if (nIdx < 0) nIdx = static_cast<int>(_normals.size()) + nIdx;
                else nIdx -= 1;
            }

            groups.push_back({vIdx, nIdx});
        }

        const int vsize = static_cast<int>(_vertices.size());
        const int nsize = static_cast<int>(_normals.size());

        for (size_t i = 1; i + 1 < groups.size(); i++) {
            int idxs[3] = { groups[0].v, groups[i].v, groups[i+1].v };
            int nidxs[3] = { groups[0].n, groups[i].n, groups[i+1].n };

            if (idxs[0] < 0 || idxs[0] >= vsize || idxs[1] < 0 || idxs[1] >= vsize || idxs[2] < 0 || idxs[2] >= vsize)
                continue;

            RayTracer::Triangle tri(_vertices[idxs[0]], _vertices[idxs[1]], _vertices[idxs[2]]);

            for (int j = 0; j < 3; ++j) {
                if (nidxs[j] >= 0 && nidxs[j] < nsize) {
                    tri.vn[j] = _normals[nidxs[j]];
                } else {
                    tri.vn[j] = tri.normal; 
                }
            }
            tri.normal = (tri.v[1] - tri.v[0]).cross(tri.v[2] - tri.v[0]).normalize();
            
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
        _uvs.clear();

        static const std::map<std::string, std::function<void(std::istringstream&)>> handlers = {
            {"v",  [this](auto& ss) { this->parseVertex(ss); }},
            {"vn", [this](auto& ss) { this->parseNormal(ss); }},
            {"vt", [this](auto& ss) { this->parseUVs(ss); }},
            {"f",  [this, &triangles](auto& ss) { this->parseFace(ss, triangles); }}
        };

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream ss(line);
            std::string type;
            if (!(ss >> type)) continue;

            auto it = handlers.find(type);
            if (it != handlers.end()) {
                it->second(ss);
            }
        }
        
        return triangles;
    }
}
