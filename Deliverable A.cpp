#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath> // for std::round
#include <tuple> // for std::tuple
#include <array> // for std::array

//--------------- INPUTS ------------//

const double epsV = 1e-10; // Vertex Tolerance used for quantisation

// Store vertices in SoA form //

std::vector<double> Vx;
std::vector<double> Vy;
std::vector<double> Vz;

//Each tetrahedron is 4 vertex IDs

struct Tet{
    int v[4];  // indices for the tetrahedron vertices
};

//All tets
std::vector<Tet> T;

//Quantised coordinates structure (qx, qy, qz)
typedef std::tuple<long long, long long, long long> Key;

// Function to add the quantised vertices to a Hash table
struct KeyHash {
    std::size_t operator() (const Key& k) const {
        
        long long a = std::get<0>(k);  //fetching the coordinates 
        long long b = std::get<1>(k);
        long long c = std::get<2>(k);

        std::size_t h1 = std::hash<long long>() (a); //hashing the coordinates
        std::size_t h2 = std::hash<long long>() (b);
        std::size_t h3 = std::hash<long long>() (c);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

// Function to check equality of vertex coordinates 

struct KeyEq {
    bool operator()(const Key& a, const Key&b) const{
        return std::get<0>(a) == std::get<0>(b) &&
               std::get<1>(a) == std::get<1>(b) &&
               std::get<2>(a) == std::get<2>(b);
    }
};

//Map from quantised key to vertex ID
std::unordered_map<Key , int, KeyHash, KeyEq> key_to_vertex_id;

// Function that creates the vertex ID

int getVertexID(double x,double y, double z) {

    //Quantize coordinates
    
    long long qx = static_cast<long long>(std::llround(x/epsV));
    long long qy = static_cast<long long>(std::llround(y/epsV));
    long long qz = static_cast<long long>(std::llround(z/epsV));

    Key key (qx,qy,qz); // Putting the quantised coordinates back together

    // To check if this key already exists
    auto it = key_to_vertex_id.find(key);
    if (it != key_to_vertex_id.end()) {
        return it->second;
    }

    //If not found, create a new vertex ID
    int new_id = static_cast<int>(Vx.size());

    Vx.push_back(x);
    Vy.push_back(y);
    Vz.push_back(y);

    key_to_vertex_id[key] = new_id; //Stores the key in the map is new ID

    return new_id; // Returns the new vertex ID
}

//------------ MAIN CODE --------------//

int main(){

//Open the input file
std::ifstream in("mesh_tets_expanded.txt");
if (!in) {
    std:: cerr << "Error: could not open mesh_tets_expanded.txt\n";
    return 1;
}

std:: string line; // holds one line of text at a time
int line_number = 0; // Tracks which line you are on

//Read file line by line
while (std::getline(in,line)) {
    ++line_number;

    //Skip empty lines
    if (line.empty()) {
        continue;
    }

    //Skip comment lines that start with "#"
    if (line[0] == '#') {
        continue;
    }

    // Prepare to read 12 doubles from this line 
    std::istringstream iss(line);
    double x[4], y[4], z[4];

    // Reading the vertices in a tetrahedron
    if (!(iss >> x[0] >> y[0] >> z[0]
              >> x[1] >> y[1] >> z[1]
              >> x[1] >> y[1] >> z[1])) {

                std::cerr << "Warning: Could not parse numeric data on line"
                          << line_number << "\n";
                continue;
              }

    // Vertex ID created for each vertex 
    Tet tet;
    for (int i=0; i < 4; ++i) {
        tet.v[i] = getVertexID(x[i],y[i],z[i]);
    }      

    //Store the tet
    T.push_back(tet);
}

in.close(); //closes the input file

//Print number of unique vertices and tets so you can see the result
std:: cout << "Number of unique vertices: " <<Vx.size() << "\n";
std:: cout << "Number of tets: " << T.size() << "\n";

//Print the first tet and its vertices 
if (!T.empty()) {
    std::cout << "First tet vertex IDs: "
              << T[1].v[0] << " "
              << T[1].v[1] << " "
              << T[1].v[2] << " "
              << T[1].v[3] << "\n";
    
    int id0 = T[1].v[0];
    std::cout << "Vertex " << id0 << " coords:"
              << Vx[id0] << " " << Vy[id0] << " " << Vz[id0] << "\n";
}

    return 0;
}

 


