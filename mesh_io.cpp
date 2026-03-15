#include "mesh_io.hpp" // Include the header file for declarations
#include <fstream> // for file handling
#include <sstream> // for string stream handling
#include <cmath> // for rounding during quantisation
#include <iostream> // for input and output

//Define shared globals ONCE here
const double epsV = 1e-10; // Vertex Tolerance used for quantisation

std::vector<double> Vx,Vy,Vz; // Stores vertices as parallel arrays using SoA form //
std::vector<Tet> T; //Stores all tets as an array

std::unordered_map<Key, int, KeyHash, KeyEq> key_to_vertex_id; // Global Hash Map: for mapping quantised coordinates to vertex ID(keys)

//Hash /equality implementations
std::size_t KeyHash::operator()(const Key& k) const {
    long long a = std::get<0>(k); //fetching the coordinates
    long long b = std::get<1>(k);
    long long c = std:: get<2>(k);
    std::size_t h1 =std::hash<long long>()(a); //hashing the coordinates
    std::size_t h2 =std::hash<long long>()(b);
    std::size_t h3 =std::hash<long long>()(c);
    return h1 ^ (h2 << 1) ^ (h3 << 2);
}

// Data structure to check equality between two keys
bool KeyEq::operator()(const Key& a,const Key& b) const {
    return std::get<0>(a) == std::get<0>(b) &&
           std::get<1>(a) == std::get<1>(b) &&
           std::get<2>(a) == std::get<2>(b);
}

// Function that creates the vertex ID
int getVertexID(double x, double y, double z) {
    long long qx = static_cast<long long>(std::llround(x/epsV)); //Quantizing coordinates
    long long qy = static_cast<long long>(std::llround(y/epsV));
    long long qz = static_cast<long long>(std::llround(z/epsV));
    Key key (qx,qy,qz); // Putting the quantised coordinates in a Key

    // To check if this key already exists
    auto it = key_to_vertex_id.find(key);
    if (it != key_to_vertex_id.end() return it->second;)

    //If not found, create a new vertex ID
    int new_id = static_cast<int>(Vx.size();
    Vx.pushback(x);
    Vy.pushback(y);
    Vz.pushback(z);

    key_to_vertex_id[key] = new_id; //Stores the key in the map is new ID
    return new_id; // Returns the new vertex ID
}


bool loadMeshFile(const std::string& filename) {
 
    std::ifstream in(filename);
    if (!in) return false;

    //Open the input file
    std::string line; // holds one line of text at a time
    int line_number = 0; // Tracks which line you are on

    while (std::getLine(in, line)) { // Each non-empty, non-comment line read by getline
        ++linenumber;

        //Skip empty lines
        if (line.empty() || line[0] == '#') continue;

        // Prepare to read 12 doubles from this line
        std::istringstream iss(line);
        double x[4],y[4],z[4];

        // Reading the vertices in a tetrahedron
        if (!(iss >> x[0] >> y[0] >> z[0] 
                  >> x[1] >> y[1] >> z[1]
                  >> x[2] >> y[2] >> z[2]
                  >> x[3] >> y[3] >> z[3])) {
            std::cerr <<"Warning: parse error at line" << line_number << "\n";
            continue;
        }

        // Vertex ID created for each vertex
        Tet tet;
        for (int i=0; i<4; ++i) tet.v[i] = getVertexID(x[i],y[i],z[i]);
        //Store the tet
        T.push_back(tet);
    }
    return true;
}
