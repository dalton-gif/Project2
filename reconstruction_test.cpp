// CPP script function to verify the constrcuted mesh in main with the raw text file

#include<iostream>                  // std for Input & output
#include<fstream>                   // Read and Write Files
#include<sstream>                   // Handling strings of Data from txt file
#include<cmath>                     // Useful for rounding off during quantisation
#include "reconstruction_test.hpp"  // Header files for reconstruction_test
#include "mesh_io.hpp"              // Header file of mesh_io

//Refering to the variables defined in Deliverable A.cpp using extern

extern const double epsV;           // Vertex Tolerance used for quantisation

const double coordTol = 1e-8;       // Tolereance for verifying the vertex co-ordinates

extern std::vector<double> Vx;
extern std::vector<double> Vy;
extern std::vector<double> Vz;

//Quantised coordinates structure (qx, qy, qz)
typedef std::tuple<long long, long long, long long> Key;

// // Function to add the quantised vertices to a Hash table
// struct KeyHash {
//     std::size_t operator() (const Key& k) const {
        
//         long long a = std::get<0>(k);  //fetching the coordinates 
//         long long b = std::get<1>(k);
//         long long c = std::get<2>(k);

//         std::size_t h1 = std::hash<long long>() (a); //hashing the coordinates
//         std::size_t h2 = std::hash<long long>() (b);
//         std::size_t h3 = std::hash<long long>() (c);

//         return h1 ^ (h2 << 1) ^ (h3 << 2);
//     }
// };

// // Function to check equality of vertex coordinates 

// struct KeyEq {
//     bool operator()(const Key& a, const Key&b) const{
//         return std::get<0>(a) == std::get<0>(b) &&
//                std::get<1>(a) == std::get<1>(b) &&
//                std::get<2>(a) == std::get<2>(b);
//     }
// };


extern std::unordered_map<Key, int, KeyHash, KeyEq> key_to_vertex_id;


/*
------------------------------
Reconstruction test Function
------------------------------
Function working sequence :
1) Re-reads the Vertices from expanded mesh file
2) For each vertex, re-creates the quantized key
3) Looks up the vertex ID from Map created by Hash mapping
4) Retrives stored co-ordinates from Vx, Vy, Vz 
5) Compare original (exapnded tx file) vs Stored Co-ordinates (unique vertices)
6) Reports max error, RMS error and failed count

-------------------------------------------------------
*/

void reconstructionTest(const std::string& filename)
{
    std::ifstream in (filename);

    if(!in){
        std::cerr<< "Error! Could not open the file for reconstruction test" 
                 <<std::endl;

        return;
    }


 // Defining variables to track the reconstruction validation stats

 double maxError = 0.0;                  // largest error amongst the co-ordinates while verifying
 double sumSquaredError = 0.0;           // Root mean Square error
 long long totalVerticesChecked = 0;     // Count of verified vertices
 int failedTets = 0;                     // Count of Tets that failed to get verified with given tolerance

  std::string line;
  int line_number = 0;               // Tracks which line you are on

   /*---------------------------------------------------------
        STEP 1 -> re-reading the vertices from exapnded TXT file
        ---------------------------------------------------------*/

 while (std::getline(in,line))            // read file line by line
  {
    ++line_number;
    if(line.empty()) continue;          // Skip empty lines
    if(line[0] == '#') continue;        // Skip comment lines atrting with #

    std::istringstream iss(line);       
    double x[4], y[4], z[4];            // Hold the co-ordinates in a line

    if(!(iss >> x[0] >>y[0] >>z[0]
             >> x[1] >>y[1] >>z[1]
             >> x[2] >>y[2] >>z[2]
             >> x[3] >>y[3] >>z[3]))
    {
        std::cerr << "Warning: Could not parse numeric data on line while reading from Txt file in line number"
                  << line_number << std::endl;
        continue;

    }

    bool tetFailed = false;             // Flag for this Tet

    for(int i=0; i<4; ++i)
    {
        /*---------------------------------------------------------
        STEP 2 -> Quantising Co-ordinates from exapnded TXT file
        ---------------------------------------------------------*/

        long long qx = std::llround(x[i]/epsV);
        long long qy = std::llround(y[i]/epsV);
        long long qz = std::llround(z[i]/epsV);
        
        Key key(qx, qy, qz);


         /*---------------------------------------------------------
        STEP 3 -> Look Up vertex ID in Hash Map
        ---------------------------------------------------------*/

        auto it = key_to_vertex_id.find(key);

        if(it == key_to_vertex_id.end())
        {
            tetFailed = true;
            continue;
        }

        int id = it->second;        // ID is assigned from Second value of it ((qx,qy,qz), ID)

        /*----------------------------------------------------------------------
        STEP 4 -> Retrive the Co-ordinates from Unique vertices using Vertex ID
        -----------------------------------------------------------------------*/

        double rx = Vx[id];
        double ry = Vy[id];
        double rz = Vz[id];         // Reconstructed x,y,z co-ordinates

         /*---------------------------------------------------------
        STEP 5 -> Compare Original VS Stored(reconstructed) Co-ordinates
        ---------------------------------------------------------*/

        double ex = std::fabs(x[i]-rx);
        double ey = std::fabs(y[i]-ry);
        double ez = std::fabs(z[i]-rz);

        //double vertexError = std::max(ex, std:: max(ey,ez));   // Computing max error in co-ordinates
        double vertexError = std::sqrt(ex*ex + ey*ey + ez*ez);   // Computing max error in co-ordinates

        if(vertexError > maxError) maxError = vertexError;     // Updating the Maximum vertex error value

        sumSquaredError += vertexError*vertexError;            // Computing RMS error
        totalVerticesChecked++;

        if(vertexError > coordTol) tetFailed = true;           // Identifying Tet as failed if error is beyond defined tolerance

    }

    if (tetFailed) failedTets++;

  }

  in.close();

  double rmsError = 0.0;

  if(totalVerticesChecked>0)
  rmsError = std::sqrt(sumSquaredError / totalVerticesChecked);

  /*Print Summary*/

  std::cout <<std::endl <<"=============== Reconstruction test Summary ===============" <<std::endl;
  std::cout <<"Max Error         :" << maxError <<std::endl;
  std::cout <<"RMS Error         :" << rmsError <<std::endl;
  std::cout <<"Failed Tets       :" << failedTets <<std::endl;
  std::cout <<"Vertices tested   :" << totalVerticesChecked <<std::endl;
  std::cout <<"========================================================================" <<std::endl;
  
}