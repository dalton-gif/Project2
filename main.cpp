#include <iostream>
#include <algorithm>

#include "mesh_io.hpp"
#include "queries.hpp"
#include "validation.hpp"
#include "reconstruction_test.hpp"
#include "mesh_export.hpp"

int main(){
    const std::string filename = "mesh_tets_expanded.txt";

    if (!loadMeshFile(filename)){
        std::cerr <<"Error: coudl not open " <<filename << "\n";
        return 1;
    }
    
    std::cout << "Unique vertices: " << Vx.size() << "\n";
    std::cout << "Tets:            " << T.size() << "\n\n";

    // Deliverable B
    validate_index_ranges(true);
    count_inverted_and_degenerate();
    detect_duplicate_tets();
    face_manifold_check();
    
    // Deliverable C
    uint32_t nV = static_cast<uint32_t>(Vx.size());
    VertexStarCSR star = buildVertexStarCSR(nV, T);
    Connectivity conn  = buildNeighborsAndBoundary(T);

    uint32_t v = 10;
    auto [s, e] = star.range(v);
    std::cout << "\n[Deliverable C]\n";
    // Tets in contact with vertex
    std::cout << "Vertex " << v << " touches " << (e - s) << " tets\n";
    // Faces of Tet
    if (!T.empty()) {
        std::cout << "Neighbors of tet 0 (4 faces): ";
        for (int f = 0; f < 4; ++f) std::cout << conn.neighbors[0][f] << " ";
        std::cout << "\n";
    }
    std::cout << "Boundary faces:  " << conn.boundary_faces.size() << "\n";
    // Edge Star Section
    if (!T.empty()) {
        EdgeStarIndex edgeIdx = buildEdgeStarIndex(T);
        uint32_t ei = static_cast<uint32_t>(T[0].v[0]);
        uint32_t ej = static_cast<uint32_t>(T[0].v[1]);
        auto edgeTets = edgeIdx.query(ei, ej);
        std::cout << "edgeStar(" << ei << "," << ej << ") size: "
                  << (edgeTets ? edgeTets->size() : 0) << "\n";
    }
    // Point inside mesh?
    Vec3d p{0.0, 0.0, 0.0};
    bool inside = isPointInsideMeshRayCast(p, Vx, Vy, Vz, conn.boundary_faces);
    std::cout << "Point (0,0,0) inside mesh? " << (inside ? "YES" : "NO") << "\n";
    
    //----------------------------------------------------------------------------------
    // Deliverable D :: Reconstruction correctness test is called through below function
    //----------------------------------------------------------------------------------
    // reconstructionTest -> Defined in reconstruction_test.cpp
    
    reconstructionTest(filename);

    // Exporting to .msh File

    Mesh mesh;                  // Mesh is initialised in mesh_export.hpp

        mesh.x = Vx;            // Assigning X- coord Vector to mesh.x
        mesh.y = Vy;            // Assigning Y- coord Vector to mesh.y
        mesh.z = Vz;            // Assigning Z- coord Vector to mesh.z

    for (const Tet& tet : T)
    {   mesh.t0.push_back(tet.v[0]);  // Add first vertex index of Tet to mesh.to 
        mesh.t1.push_back(tet.v[1]);  // Add second vertex index of Tet to mesh.t1 
        mesh.t2.push_back(tet.v[2]);  // Add third vertex index of Tet to mesh.t2 
        mesh.t3.push_back(tet.v[3]);  // Add fourth vertex index of Tet to mesh.t3 
    }
    
    // export.MSH -> defined in mesh_export.cpp
    
    mesh.exportMSH("New_exported_mesh.msh");

}
