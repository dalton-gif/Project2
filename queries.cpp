#include "queries.hpp"

#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include <cmath>

// =============================================================
// Deliverable C — Query / Search Tools for Tetrahedral Mesh
// Inputs:  T = list of tetrahedra (each tet has 4 vertex IDs)
// Outputs: CSR vertex-star, tet neighbors, boundary faces,
//          edge-star index, point-in-mesh test.
// =============================================================

// -----------------------------
// 1) Vertex Star via CSR
// -----------------------------
// CSR format stores vertex->tet adjacency compactly:
// - offsets[v]..offsets[v+1]-1 indexes into adj
// - adj stores tet IDs incident to vertex v
VertexStarCSR buildVertexStarCSR(uint32_t num_vertices, const std::vector<Tet>& T) {
    VertexStarCSR out;
    out.offsets.assign(num_vertices + 1, 0);

    // Step 1: Count incident tets per vertex
    for (uint32_t tid = 0; tid < (uint32_t)T.size(); ++tid) {
        for (int k = 0; k < 4; ++k) {
            uint32_t v = (uint32_t)T[tid].v[k];
            out.offsets[v + 1]++; // store counts shifted by 1
        }
    }

    // Step 2: Prefix sum turns counts into offsets 
    for (uint32_t v = 0; v < num_vertices; ++v) {
        out.offsets[v + 1] += out.offsets[v];
    }

    // Step 3: Fill adjacency list 
    out.adj.assign(out.offsets[num_vertices], 0);

    // cursor is a write-pointer per vertex (starts at offsets[v])
    std::vector<uint32_t> cursor = out.offsets;

    // Write tet IDs into the correct place in adj
    for (uint32_t tid = 0; tid < (uint32_t)T.size(); ++tid) {
        for (int k = 0; k < 4; ++k) {
            uint32_t v = (uint32_t)T[tid].v[k];
            out.adj[cursor[v]++] = tid;
        }
    }

    return out;
}

// -----------------------------
// 2) Face keys (internal helpers)
// -----------------------------
// FaceKey identifies a triangle face by its 3 vertex IDs.
// We store them sorted so that different orderings match.
struct FaceKey {
    uint32_t a, b, c; // sorted a<=b<=c
    bool operator==(const FaceKey& o) const { return a == o.a && b == o.b && c == o.c; }
};

// Hash for FaceKey so it can be used in unordered_map
struct FaceKeyHash {
    size_t operator()(const FaceKey& f) const noexcept {
        size_t h = std::hash<uint32_t>()(f.a);
        h ^= std::hash<uint32_t>()(f.b) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint32_t>()(f.c) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

// Create canonical face key by sorting vertex IDs
static inline FaceKey makeFaceKey(uint32_t i, uint32_t j, uint32_t k) {
    if (i > j) std::swap(i, j);
    if (j > k) std::swap(j, k);
    if (i > j) std::swap(i, j);
    return FaceKey{i, j, k};
}

// Return the 4 faces of a tet (as canonical sorted triples)
static inline std::array<FaceKey, 4> tetFaces(const Tet& t) {
    uint32_t a = (uint32_t)t.v[0];
    uint32_t b = (uint32_t)t.v[1];
    uint32_t c = (uint32_t)t.v[2];
    uint32_t d = (uint32_t)t.v[3];

    // Faces: (a,b,c), (a,b,d), (a,c,d), (b,c,d)
    return {
        makeFaceKey(a, b, c),
        makeFaceKey(a, b, d),
        makeFaceKey(a, c, d),
        makeFaceKey(b, c, d)
    };
}

// -----------------------------
// 3) Neighbors + Boundary faces
// -----------------------------
// Two tets are neighbors if they share a face.
// neighbors[tid][f] stores the tet across face f, or -1 if boundary.
// boundary_faces stores faces that occur only once in the mesh.
Connectivity buildNeighborsAndBoundary(const std::vector<Tet>& T) {
    Connectivity out;
    out.neighbors.assign(T.size(), std::array<int32_t, 4>{-1, -1, -1, -1});

    // FaceOwner records who first saw a face: (tet id, local face index)
    struct FaceOwner { uint32_t tid; uint8_t face; };
    std::unordered_map<FaceKey, FaceOwner, FaceKeyHash> faceMap;
    faceMap.reserve(T.size() * 2);

    //  Build neighbor relations using face hash map
    for (uint32_t tid = 0; tid < (uint32_t)T.size(); ++tid) {
        auto faces = tetFaces(T[tid]);

        for (uint8_t f = 0; f < 4; ++f) {
            const FaceKey& key = faces[f];
            auto it = faceMap.find(key);

            if (it == faceMap.end()) {
                // First time this face appears
                faceMap.emplace(key, FaceOwner{tid, f});
            } else {
                // Second time: link the two tets as neighbors
                uint32_t otherTid = it->second.tid;
                uint8_t otherF = it->second.face;

                out.neighbors[tid][f] = (int32_t)otherTid;
                out.neighbors[otherTid][otherF] = (int32_t)tid;
            }
        }
    }

    //  Faces with no neighbor are boundary faces
    out.boundary_faces.reserve(T.size());
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

// ==================================
// Extra Query 1: edgeStar(i, j)
// ==================================
// edgeStar(i,j) returns all tets that contain both vertices i and j
// We build an index: edge -> list of incident tets.
static inline uint64_t packEdgeKey(uint32_t i, uint32_t j) {
    if (i > j) std::swap(i, j);                 // canonical order
    return (uint64_t(i) << 32) | uint64_t(j);   // pack into one 64-bit key
}

EdgeStarIndex buildEdgeStarIndex(const std::vector<Tet>& T) {
    EdgeStarIndex idx;
    idx.edge_to_tets.reserve(T.size() * 3); // rough reserve

    for (uint32_t tid = 0; tid < (uint32_t)T.size(); ++tid) {
        uint32_t a = (uint32_t)T[tid].v[0];
        uint32_t b = (uint32_t)T[tid].v[1];
        uint32_t c = (uint32_t)T[tid].v[2];
        uint32_t d = (uint32_t)T[tid].v[3];

        // Each tet has 6 edges
        idx.edge_to_tets[packEdgeKey(a,b)].push_back(tid);
        idx.edge_to_tets[packEdgeKey(a,c)].push_back(tid);
        idx.edge_to_tets[packEdgeKey(a,d)].push_back(tid);
        idx.edge_to_tets[packEdgeKey(b,c)].push_back(tid);
        idx.edge_to_tets[packEdgeKey(b,d)].push_back(tid);
        idx.edge_to_tets[packEdgeKey(c,d)].push_back(tid);
    }

    return idx;
}

const std::vector<uint32_t>* EdgeStarIndex::query(uint32_t i, uint32_t j) const {
    uint64_t key = packEdgeKey(i, j);
    auto it = edge_to_tets.find(key);
    if (it == edge_to_tets.end()) return nullptr;
    return &it->second;
}

// ==================================
// Extra Query 2: isPointInsideMesh(p)
// ==================================
// Uses ray casting: shoot a ray in +X direction from point p.
// Count triangle intersections with boundary surface.
// Odd intersections => inside; even => outside.
// Note: works best if boundary surface is closed (watertight).

// Ray-triangle intersection (Möller–Trumbore), ray dir fixed to +X
static inline bool rayIntersectsTriangle_PosX(
    const Vec3d& p,
    const Vec3d& v0,
    const Vec3d& v1,
    const Vec3d& v2,
    double eps = 1e-12
) {
    const Vec3d dir{1.0, 0.0, 0.0}; // +X ray direction

    // Triangle edges
    Vec3d e1{v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
    Vec3d e2{v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};

    Vec3d pvec{
        dir.y * e2.z - dir.z * e2.y,
        dir.z * e2.x - dir.x * e2.z,
        dir.x * e2.y - dir.y * e2.x
    };

    double det = e1.x * pvec.x + e1.y * pvec.y + e1.z * pvec.z;
    if (std::fabs(det) < eps) return false; // parallel or degenerate

    double invDet = 1.0 / det;

    // u coordinate
    Vec3d tvec{p.x - v0.x, p.y - v0.y, p.z - v0.z};
    double u = (tvec.x * pvec.x + tvec.y * pvec.y + tvec.z * pvec.z) * invDet;
    if (u < -eps || u > 1.0 + eps) return false;

    // v coordinate
    Vec3d qvec{
        tvec.y * e1.z - tvec.z * e1.y,
        tvec.z * e1.x - tvec.x * e1.z,
        tvec.x * e1.y - tvec.y * e1.x
    };

    double v = (dir.x * qvec.x + dir.y * qvec.y + dir.z * qvec.z) * invDet;
    if (v < -eps || u + v > 1.0 + eps) return false;

    // intersection distance along the ray
    double t = (e2.x * qvec.x + e2.y * qvec.y + e2.z * qvec.z) * invDet;
    return (t > eps);
}

bool isPointInsideMeshRayCast(
    const Vec3d& p_in,
    const std::vector<double>& Vx,
    const std::vector<double>& Vy,
    const std::vector<double>& Vz,
    const std::vector<std::array<uint32_t,3>>& boundary_faces
) {
    if (Vx.empty() || boundary_faces.empty()) return false;

    // jitter to reduce ambiguity when ray hits edges/vertices exactly
    Vec3d p = p_in;
    p.x += 1e-12;

    // Quick reject using bounding box of vertices
    double minx = Vx[0], maxx = Vx[0];
    double miny = Vy[0], maxy = Vy[0];
    double minz = Vz[0], maxz = Vz[0];
    for (size_t i = 1; i < Vx.size(); ++i) {
        minx = std::min(minx, Vx[i]); maxx = std::max(maxx, Vx[i]);
        miny = std::min(miny, Vy[i]); maxy = std::max(maxy, Vy[i]);
        minz = std::min(minz, Vz[i]); maxz = std::max(maxz, Vz[i]);
    }
    if (p.x < minx || p.x > maxx || p.y < miny || p.y > maxy || p.z < minz || p.z > maxz) {
        return false;
    }

    // Count ray intersections with boundary triangles
    int hits = 0;
    for (const auto& tri : boundary_faces) {
        uint32_t ia = tri[0], ib = tri[1], ic = tri[2];
        Vec3d a{Vx[ia], Vy[ia], Vz[ia]};
        Vec3d b{Vx[ib], Vy[ib], Vz[ib]};
        Vec3d c{Vx[ic], Vy[ic], Vz[ic]};

        if (rayIntersectsTriangle_PosX(p, a, b, c)) hits++;
    }

    // Odd/even rule
    return (hits % 2) == 1;
}