#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <cstdint>

// Your tet struct from Deliverable A
struct Tet {
    int v[4];
};

// -----------------------------
// 1) Vertex Star via CSR
// -----------------------------
struct VertexStarCSR {
    // offsets size = num_vertices + 1
    // adj size = 4 * num_tets
    std::vector<uint32_t> offsets;
    std::vector<uint32_t> adj;

    // Return the [start,end) range in adj for vertex v
    std::pair<uint32_t, uint32_t> range(uint32_t v) const {
        return { offsets[v], offsets[v + 1] };
    }
};

// Build CSR vertex star from tets
VertexStarCSR buildVertexStarCSR(uint32_t num_vertices, const std::vector<Tet>& T) {
    VertexStarCSR out;
    out.offsets.assign(num_vertices + 1, 0);

    // Step 1: count degrees (how many tets touch each vertex)
    for (uint32_t tid = 0; tid < (uint32_t)T.size(); ++tid) {
        for (int k = 0; k < 4; ++k) {
            uint32_t v = (uint32_t)T[tid].v[k];
            out.offsets[v + 1]++; // degree count stored shifted by 1
        }
    }

    // Step 2: prefix sum to turn counts into offsets
    for (uint32_t v = 0; v < num_vertices; ++v) {
        out.offsets[v + 1] += out.offsets[v];
    }

    // Step 3: fill adjacency list
    out.adj.assign(out.offsets[num_vertices], 0);

    // We'll use a working copy of offsets as write cursors
    std::vector<uint32_t> cursor = out.offsets;

    for (uint32_t tid = 0; tid < (uint32_t)T.size(); ++tid) {
        for (int k = 0; k < 4; ++k) {
            uint32_t v = (uint32_t)T[tid].v[k];
            out.adj[cursor[v]++] = tid;
        }
    }

    return out;
}

// -----------------------------
// 2) Face key for neighbor/boundary
// -----------------------------
struct FaceKey {
    uint32_t a, b, c; // sorted a<=b<=c

    bool operator==(const FaceKey& o) const {
        return a == o.a && b == o.b && c == o.c;
    }
};

struct FaceKeyHash {
    size_t operator()(const FaceKey& f) const noexcept {
        // Simple combine
        size_t h = std::hash<uint32_t>()(f.a);
        h ^= std::hash<uint32_t>()(f.b) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint32_t>()(f.c) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

static inline FaceKey makeFaceKey(uint32_t i, uint32_t j, uint32_t k) {
    // sort the triple
    if (i > j) std::swap(i, j);
    if (j > k) std::swap(j, k);
    if (i > j) std::swap(i, j);
    return FaceKey{i, j, k};
}

// Each tet has 4 faces; we label them 0..3.
// For tet (a,b,c,d), common convention:
static inline std::array<FaceKey, 4> tetFaces(const Tet& t) {
    uint32_t a = (uint32_t)t.v[0];
    uint32_t b = (uint32_t)t.v[1];
    uint32_t c = (uint32_t)t.v[2];
    uint32_t d = (uint32_t)t.v[3];

    return {
        makeFaceKey(a, b, c), // face opposite d
        makeFaceKey(a, b, d), // opposite c
        makeFaceKey(a, c, d), // opposite b
        makeFaceKey(b, c, d)  // opposite a
    };
}

// -----------------------------
// 3) Neighbors + Boundary faces
// -----------------------------
struct Connectivity {
    // neighbors[tid][f] = neighboring tet across face f, or -1 if boundary
    std::vector<std::array<int32_t, 4>> neighbors;

    // boundary faces as triples of vertex IDs (canonical sorted)
    std::vector<std::array<uint32_t, 3>> boundary_faces;
};

Connectivity buildNeighborsAndBoundary(const std::vector<Tet>& T) {
    Connectivity out;
    out.neighbors.assign(T.size(), std::array<int32_t, 4>{-1, -1, -1, -1});

    // Map face -> first occurrence (tetID, localFaceID)
    struct FaceOwner { uint32_t tid; uint8_t face; };

    std::unordered_map<FaceKey, FaceOwner, FaceKeyHash> faceMap;
    faceMap.reserve(T.size() * 2); // rough reserve

    // First pass: fill map and link neighbors
    for (uint32_t tid = 0; tid < (uint32_t)T.size(); ++tid) {
        auto faces = tetFaces(T[tid]);

        for (uint8_t f = 0; f < 4; ++f) {
            const FaceKey& key = faces[f];
            auto it = faceMap.find(key);

            if (it == faceMap.end()) {
                // first time we see this face
                faceMap.emplace(key, FaceOwner{tid, f});
            } else {
                // second time: connect two tets as neighbors
                uint32_t otherTid = it->second.tid;
                uint8_t otherF = it->second.face;

                out.neighbors[tid][f] = (int32_t)otherTid;
                out.neighbors[otherTid][otherF] = (int32_t)tid;

                // faceMap.erase(it);
            }
        }
    }

    // Second pass: boundary faces = faces that have no neighbor
    out.boundary_faces.reserve(T.size()); // rough

    for (uint32_t tid = 0; tid < (uint32_t)T.size(); ++tid) {
        auto faces = tetFaces(T[tid]);

        for (uint8_t f = 0; f < 4; ++f) {
            if (out.neighbors[tid][f] == -1) {
                const FaceKey& key = faces[f];
                out.boundary_faces.push_back({key.a, key.b, key.c});
            }
        }
    }

    return out;
}