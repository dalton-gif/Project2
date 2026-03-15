#pragma once
#include <vector>
#include <string>
#include <cstdint>

/*Initilaising a Class Mesh to get Vectors of vertices and 
  Tets with Connectivity which is linked/defined in main.cpp*/

class Mesh
{
public:

    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> z;

    std::vector<uint32_t> t0;
    std::vector<uint32_t> t1;
    std::vector<uint32_t> t2;
    std::vector<uint32_t> t3;

    // Calling exportMSH function defined in export_mesh.cpp

    void exportMSH(const std::string& filename);
};
