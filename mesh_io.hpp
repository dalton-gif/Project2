#pragma once
#include <vector>
#include <tuple>
#include <unordered_map>
#include <string>

// ----- Shared tolerance (defined once in mesh_io.cpp)
extern const double epsV;

// -----Mesh Storage (defined once in mesh_io.cpp)
extern std::vector<double> Vx;
extern std::vector<double> Vy;
extern std::vector<double> Vz;

struct Tet { int v[4]; };
extern std::vector<Tet> T;

// ----- Quantized key + map (defined once in mesh_io.cpp)
using Key = std::tuple<long long, long long, long long>;

struct KeyHash {
    std::size_t operator()(const Key& b) const;
};

struct KeyEq {
    bool operator()(const Key& a, const Key& b) const;
};

extern std::unordered_map<Key, int, KeyHash, KeyEq> key_to_vertex_id;

//Functions (defined in mesh_io.cpp)
int getVertexID(double x, double y, double z);
bool loadMeshFile (const std::string& filename);
void clearMesh();
