#include "io/OBJParser.hpp"
#include <fstream>
#include <cstdlib>

namespace OBJParser {

    static IndexGroup parseIndices(const char* token)
    {
        char* end;
        int v = (int)std::strtol(token, &end, 10);
        int t = -1, n = -1;

        if (*end == '/') {
            const char* p = end + 1;
            if (*p != '/') {
                t = (int)std::strtol(p, &end, 10);
                p = end;
            }
            if (*p == '/') {
                ++p;
                if (*p)
                    n = (int)std::strtol(p, nullptr, 10);
            }
        }
        return {v, t, n};
    }

    static const char* skipSpaces(const char* p)
    {
        while (*p == ' ' || *p == '\t') ++p;
        return p;
    }

    static const char* nextToken(const char* p, char* buf, int bufSize)
    {
        p = skipSpaces(p);
        int i = 0;
        while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && i < bufSize - 1)
            buf[i++] = *p++;
        buf[i] = '\0';
        return p;
    }

    std::vector<RayTracer::Triangle> OBJParser::load(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        std::vector<RayTracer::Triangle> triangles;

        if (!file.is_open())
            return triangles;

        _vertices.clear();
        _normals.clear();
        _uvs.clear();

        static constexpr int MAX_FACE_VERTS = 16;
        IndexGroup faceGroups[MAX_FACE_VERTS];

        // Store vertex indices per triangle for smooth normal computation
        struct TriIndices { int vi[3]; int ti[3]; };
        std::vector<TriIndices> triIndices;

        std::string line;
        line.reserve(256);

        while (std::getline(file, line)) {
            const char* p = line.c_str();
            p = skipSpaces(p);

            if (*p == '#' || *p == '\0' || *p == '\r' || *p == '\n')
                continue;

            const char type0 = p[0];
            const char type1 = (type0 && p[1]) ? p[1] : '\0';

            // vertex normal: "vn"
            if (type0 == 'v' && type1 == 'n') {
                p += 2;
                char* end;
                double x = std::strtod(p, &end); p = end;
                double y = std::strtod(p, &end); p = end;
                double z = std::strtod(p, &end);
                _normals.push_back({x, y, z});
            }
            // vertex texture: "vt"
            else if (type0 == 'v' && type1 == 't') {
                p += 2;
                char* end;
                float u = std::strtof(p, &end); p = end;
                float v = std::strtof(p, &end);
                _uvs.push_back({(double)u, (double)v});
            }
            // vertex position: "v "
            else if (type0 == 'v' && (type1 == ' ' || type1 == '\t')) {
                p += 1;
                char* end;
                double x = std::strtod(p, &end); p = end;
                double y = std::strtod(p, &end); p = end;
                double z = std::strtod(p, &end);
                _vertices.push_back({x, y, z});
            }
            // face: "f"
            else if (type0 == 'f' && (type1 == ' ' || type1 == '\t')) {
                p += 1;
                int groupCount = 0;
                char token[64];

                while (groupCount < MAX_FACE_VERTS) {
                    p = skipSpaces(p);
                    if (*p == '\0' || *p == '\r' || *p == '\n') break;

                    p = nextToken(p, token, sizeof(token));
                    if (token[0] == '\0') break;

                    auto [v, t, n] = parseIndices(token);

                    const int vsize = (int)_vertices.size();
                    const int tsize = (int)_uvs.size();
                    const int nsize = (int)_normals.size();

                    if (v < 0) v = vsize + v; else v -= 1;
                    if (t != -1) { if (t < 0) t = tsize + t; else t -= 1; }
                    if (n != -1) { if (n < 0) n = nsize + n; else n -= 1; }

                    faceGroups[groupCount++] = {v, t, n};
                }

                const int vsize = (int)_vertices.size();
                const int tsize = (int)_uvs.size();

                for (int i = 1; i + 1 < groupCount; i++) {
                    const int idxs[3]  = { faceGroups[0].v, faceGroups[i].v, faceGroups[i+1].v };
                    const int tidxs[3] = { faceGroups[0].t, faceGroups[i].t, faceGroups[i+1].t };

                    if (idxs[0] < 0 || idxs[0] >= vsize ||
                        idxs[1] < 0 || idxs[1] >= vsize ||
                        idxs[2] < 0 || idxs[2] >= vsize)
                        continue;

                    RayTracer::Triangle tri(_vertices[idxs[0]], _vertices[idxs[1]], _vertices[idxs[2]]);
                    tri.normal = (tri.v[1] - tri.v[0]).cross(tri.v[2] - tri.v[0]).normalize();

                    for (int j = 0; j < 3; ++j)
                        tri.uv[j] = (tidxs[j] >= 0 && tidxs[j] < tsize) ? _uvs[tidxs[j]] : Graphic::UV{};

                    triangles.push_back(tri);
                    triIndices.push_back({{idxs[0], idxs[1], idxs[2]}, {tidxs[0], tidxs[1], tidxs[2]}});
                }
            }
        }

        // Compute smooth normals: accumulate face normals per vertex then normalize
        std::vector<Math::Vector3D> accumulated(_vertices.size(), {0.0, 0.0, 0.0});
        for (size_t i = 0; i < triangles.size(); i++) {
            for (int j = 0; j < 3; j++)
                accumulated[triIndices[i].vi[j]] += triangles[i].normal;
        }
        for (size_t i = 0; i < triangles.size(); i++) {
            for (int j = 0; j < 3; j++) {
                const Math::Vector3D& acc = accumulated[triIndices[i].vi[j]];
                triangles[i].vn[j] = acc.length() > 1e-9 ? acc.normalize() : triangles[i].normal;
            }
        }

        return triangles;
    }
}
