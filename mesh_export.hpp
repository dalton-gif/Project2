
#pragma once


#include<vector>
#include<fstream>
#include<string>
#include<cstdint>

using namespace std;

/*
Mesh Class is defined here to store vertices co-ordinates and tetrahedron
connectivity of the mesh.
--------------------------------------------------------------------------
Vectors are used because they can grow dynamically
--------------------------------------------------------------------------
*/




// // Vector for Vertex Co-ordinates
// vector<double>x;
// vector<double>y;
// vector<double>z;

// // Vectors for defining Tets and connectivity with Unsigned Integer 

// vector<uint32_t>t0;
// vector<uint32_t>t1;
// vector<uint32_t>t2;
// vector<uint32_t>t3;

void exportMSH(const std::string& filename);
