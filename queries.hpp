#pragma once
#include <vector>
#include <array>
#include <cstdint>
#include <utility>
#include <unordered_map>

#include "mesh_io.hpp"   // <-- brings Tet, and Vx/Vy/Vz 

struct VertexStarCSR {
    std::vector<uint32_t> offsets;
    std::vector<uint32_t> adj;

    std::pair<uint32_t, uint32_t> range(uint32_t v) const {
        return { offsets[v], offsets[v + 1] };
    }
};

VertexStarCSR buildVertexStarCSR(uint32_t num_vertices, const std::vector<Tet>& T);

struct Connectivity {
    std::vector<std::array<int32_t, 4>> neighbors;
    std::vector<std::array<uint32_t, 3>> boundary_faces;
};

Connectivity buildNeighborsAndBoundary(const std::vector<Tet>& T);

struct EdgeStarIndex {
    std::unordered_map<uint64_t, std::vector<uint32_t>> edge_to_tets;
    const std::vector<uint32_t>* query(uint32_t i, uint32_t j) const;
};

EdgeStarIndex buildEdgeStarIndex(const std::vector<Tet>& T);

struct Vec3d { double x, y, z; };

bool isPointInsideMeshRayCast(
    const Vec3d& p,
    const std::vector<double>& Vx,
    const std::vector<double>& Vy,
    const std::vector<double>& Vz,
    const std::vector<std::array<uint32_t,3>>& boundary_faces
);