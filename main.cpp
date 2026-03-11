#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <tuple>
#include <array>

#include "queries.hpp"

// ---- Deliverable A globals (from your teammate) ----
const double epsV = 1e-10;

std::vector<double> Vx, Vy, Vz;
std::vector<Tet> T;

using Key = std::tuple<long long, long long, long long>;

struct KeyHash {
    std::size_t operator()(const Key& k) const {
        long long a = std::get<0>(k);
        long long b = std::get<1>(k);
        long long c = std::get<2>(k);
        std::size_t h1 = std::hash<long long>()(a);
        std::size_t h2 = std::hash<long long>()(b);
        std::size_t h3 = std::hash<long long>()(c);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

struct KeyEq {
    bool operator()(const Key& a, const Key& b) const {
        return std::get<0>(a) == std::get<0>(b) &&
               std::get<1>(a) == std::get<1>(b) &&
               std::get<2>(a) == std::get<2>(b);
    }
};

std::unordered_map<Key, int, KeyHash, KeyEq> key_to_vertex_id;

int getVertexID(double x, double y, double z) {
    long long qx = static_cast<long long>(std::llround(x / epsV));
    long long qy = static_cast<long long>(std::llround(y / epsV));
    long long qz = static_cast<long long>(std::llround(z / epsV));

    Key key(qx, qy, qz);

    auto it = key_to_vertex_id.find(key);
    if (it != key_to_vertex_id.end()) {
        return it->second;
    }

    int new_id = static_cast<int>(Vx.size());
    Vx.push_back(x);
    Vy.push_back(y);
    Vz.push_back(z); // IMPORTANT FIX

    key_to_vertex_id[key] = new_id;
    return new_id;
}

bool loadMeshFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return false;

    std::string line;
    int line_number = 0;

    while (std::getline(in, line)) {
        ++line_number;
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        double x[4], y[4], z[4];

        // IMPORTANT FIX: read all 12 numbers
        if (!(iss >> x[0] >> y[0] >> z[0]
                  >> x[1] >> y[1] >> z[1]
                  >> x[2] >> y[2] >> z[2]
                  >> x[3] >> y[3] >> z[3])) {
            std::cerr << "Warning: parse error on line " << line_number << "\n";
            continue;
        }

        Tet tet;
        for (int i = 0; i < 4; ++i) {
            tet.v[i] = getVertexID(x[i], y[i], z[i]);
        }
        T.push_back(tet);
    }

    return true;
}

int main() {
    if (!loadMeshFile("mesh_tets_expanded.txt")) {
        std::cerr << "Error: could not open mesh_tets_expanded.txt\n";
        return 1;
    }

    std::cout << "Unique vertices: " << Vx.size() << "\n";
    std::cout << "Tets:            " << T.size() << "\n";

    // ---- Deliverable C: build query structures ----
    uint32_t num_vertices = static_cast<uint32_t>(Vx.size());

    VertexStarCSR star = buildVertexStarCSR(num_vertices, T);
    Connectivity conn = buildNeighborsAndBoundary(T);

    // Example query 1: vertex star
    uint32_t v = 10;
    if (v < num_vertices) {
        auto [s, e] = star.range(v);
        std::cout << "Vertex " << v << " touches " << (e - s) << " tets\n";
    }

    // Example query 2: neighbors of tet 0
    if (!T.empty()) {
        std::cout << "Neighbors of tet 0 (4 faces): ";
        for (int f = 0; f < 4; ++f) {
            std::cout << conn.neighbors[0][f] << " ";
        }
        std::cout << "\n";
    }

    // Example query 3: boundary faces
    std::cout << "Boundary faces:  " << conn.boundary_faces.size() << "\n";

    return 0;
}