#include <iostream>
#include <algorithm>

#include "mesh_io.hpp"
#include "queries.hpp"
#include "validation.hpp"
#include "reconstruction_test.hpp"
#include "mesh_export.hpp"

int main{
    const std::string filename = "mesh_tets_expanded.txt";

    if (!loadMeshFile(filename)){
        std::cerr <<"Error: coudl not open " <<filename << "\n";
        return 1;
    }
    
    std::cout << "Unique vertices: " << Vx.size() << "\n";
    std::cout << "Tets:            " << T.size() << "\n\n";

    // Deliverable B

    // Deliverable C

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
