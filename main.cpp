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

    // Deliverable D
}
