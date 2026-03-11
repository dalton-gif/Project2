#pragma once
#include <vector>
#include <array>
#include <cstdint>
#include <utility>

struct Tet { int v[4]; };

struct VertexStarCSR {
    std::vector<uint32_t> offsets;
    std::vector<uint32_t> adj;

    // ✅ Define it right here (inline)
    std::pair<uint32_t, uint32_t> range(uint32_t v) const {
        return { offsets[v], offsets[v + 1] };
    }
};

struct Connectivity {
    std::vector<std::array<int32_t, 4>> neighbors;
    std::vector<std::array<uint32_t, 3>> boundary_faces;
};

VertexStarCSR buildVertexStarCSR(uint32_t num_vertices, const std::vector<Tet>& T);
Connectivity buildNeighborsAndBoundary(const std::vector<Tet>& T);