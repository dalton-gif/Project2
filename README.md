# Project2
MEEN 704 - Mesh Project

**Style Guide**



**Tolerance** //
double epsV      = 1e-10;
double epsVol    = 1e-12;
double coordTol  = 1e-12;
Canonical keys: face = sorted(3), tet = sorted(4)

Deliverable C — Mesh Query / Search Tools (C++)

This part of the project implements Deliverable C: fast “search/query” tools for a tetrahedral mesh. It assumes Deliverable A has already reconstructed the mesh connectivity:

T: a list of tetrahedra, where each tet stores 4 vertex IDs
struct Tet { int v[4]; };

num_vertices: the number of unique vertices (for example, Vx.size() from Deliverable A)

Deliverable C mainly uses connectivity (vertex IDs in T). Vertex coordinates are not required to answer these queries.

What Deliverable C provides
1) Vertex Star Query (vertex → incident tetrahedra)

Goal: Given a vertex v, return all tetrahedra that include vertex v.

We build this using CSR (Compressed Sparse Row) storage:

offsets (size = num_vertices + 1)

adj (size ≈ 4 * num_tets)

The tet IDs touching vertex v are stored in a contiguous slice:
adj[offsets[v] ... offsets[v+1]-1]

This is faster and more memory-efficient than vector<vector<...>>.

2) Tet Neighbor Query (tet → neighboring tets across faces)

Goal: For a given tet t, return the tet on the other side of each of its 4 faces.

Each tet has 4 triangular faces (3 vertices per face). Two tets are neighbors if they share the same face. Because the same face can appear with vertex IDs in different orders, we compute a canonical face key by sorting the 3 vertex IDs. We then use a hash map to detect faces that appear twice:

first occurrence: store (tet ID, face index)

second occurrence: link the two tets as neighbors

The output is:

neighbors[t][f] = neighboring tet ID across face f, or -1 if that face is on the boundary.

3) Boundary Surface Extraction (outer triangle faces)

Goal: Identify which faces lie on the exterior boundary.

Any face that has no neighbor (neighbors[t][f] == -1) is a boundary face. We collect these into:

boundary_faces: a list of boundary triangles, each stored as 3 vertex IDs (sorted).

Files

queries.hpp — declarations of the query tools (data structures + function prototypes)

queries.cpp — implementations of:

CSR vertex-star builder

tet-neighbor builder

boundary-face extraction