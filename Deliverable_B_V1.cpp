#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <tuple>
#include <array>        // for std::array
#include <cmath>
#include <map>         // for std::map
#include <algorithm>  // for std::sort
// --------------- GLOBAL DATA (from Deliverable A style) ------------
const double epsV   = 1e-10;   // vertex quantization tolerance
const double epsVol = 1e-12;   // volume tolerance; adjust if needed

// Structure-of-Arrays for vertices
std::vector<double> Vx;
std::vector<double> Vy;
std::vector<double> Vz;

// Each tetrahedron stores 4 vertex IDs
struct Tet {
    int v[4];
};

std::vector<Tet> T;

// Quantized key type
using Key = std::tuple<long long, long long, long long>;

struct KeyHash {
    std::size_t operator()(const Key &k) const {
        long long a = std::get<0>(k);
        long long b = std::get<1>(k);
        long long c = std::get<2>(k);
        std::size_t h1 = std::hash<long long>()(a);
        std::size_t h2 = std::hash<long long>()(b);
        std::size_t h3 = std::hash<long long>()(c);
        // simple combination
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2)) ^
               (h3 + 0x9e3779b97f4a7c15ULL + (h2 << 6) + (h2 >> 2));
    }
};

struct KeyEq {
    bool operator()(const Key &a, const Key &b) const {
        return std::get<0>(a) == std::get<0>(b) &&
               std::get<1>(a) == std::get<1>(b) &&
               std::get<2>(a) == std::get<2>(b);
    }
};

std::unordered_map<Key, int, KeyHash, KeyEq> key_to_vertex_id;

// --------------- BUILD VERTEX IDS ------------
int getVertexID(double x, double y, double z) {
    long long qx = static_cast<long long>(std::llround(x / epsV));
    long long qy = static_cast<long long>(std::llround(y / epsV));
    long long qz = static_cast<long long>(std::llround(z / epsV));

    Key key(qx, qy, qz);
    auto it = key_to_vertex_id.find(key);
    if (it != key_to_vertex_id.end()) {
        return it->second;
    }

    int new_id = static_cast<int>(Vx.size());
    Vx.push_back(x);
    Vy.push_back(y);
    Vz.push_back(z);
    key_to_vertex_id[key] = new_id;
    return new_id;
}

// --------------- INDEX VALIDATION ------------
// we need check number of vertices is in range of [0, n-1]
// vid = 0..4 --> ok --> pass
// vid = -1 or vid = 5 --> false 
bool validate_index_ranges(bool verbose = true) {
    bool ok = true; //Start by assuming the mesh is valid.
    const int num_vertices = static_cast<int>(Vx.size()); //length of your x‑coordinate arry and storingin num_vertices
    const int num_tets     = static_cast<int>(T.size());  //how many tetrahedra we have.

    if (verbose) {
        std::cout << "Running index-range validation...\n"; // discplay number V,T
        std::cout << "  num_vertices = " << num_vertices
                  << ", num_tets = " << num_tets << "\n";
    }
//Starts a loop over all tetrahedra
    for (int tid = 0; tid < num_tets; ++tid) {  // loop tid --> 0 , n-1
        const Tet &tet = T[tid];       // referance to the current tetes
        for (int loc = 0; loc < 4; ++loc) {  // checking in each tete location 
            int vid = tet.v[loc];  
            if (vid < 0 || vid >= num_vertices) {   // valid rage id 0 to n-1
                ok = false; 
                std::cerr << "Index error: tet " << tid
                          << ", local vertex " << loc
                          << " has ID " << vid
                          << " which is outside [0, "
                          << num_vertices - 1 << "].\n";
            }
        }
    }

    if (verbose) {
        if (ok) {
            std::cout << "Index-range validation PASSED.\n";
        } else {
            std::cout << "Index-range validation FAILED.\n";
        }
    }

    return ok;
}

// --------------- SIGNED TET VOLUME ------------
double signedVolume(int tid) {
    const Tet &tet = T[tid];
    int ia = tet.v[0]; //Extracts the four vertex IDs of this tet:
    int ib = tet.v[1];
    int ic = tet.v[2];
    int id = tet.v[3];

    // Positions //Look up each vertex’s coordinates from the arrays Vx, Vy, Vz.
    double ax = Vx[ia], ay = Vy[ia], az = Vz[ia];
    double bx = Vx[ib], by = Vy[ib], bz = Vz[ib];
    double cx = Vx[ic], cy = Vy[ic], cz = Vz[ic];
    double dx = Vx[id], dy = Vy[id], dz = Vz[id];

    // Edge vectors //Build edge vectors from A:
    double abx = bx - ax, aby = by - ay, abz = bz - az;
    double acx = cx - ax, acy = cy - ay, acz = cz - az;
    double adx = dx - ax, ady = dy - ay, adz = dz - az;

    // cross = AC x AD //cxp,cyp,czp is that cross vector.
    double cxp = acy * adz - acz * ady;
    double cyp = acz * adx - acx * adz;
    double czp = acx * ady - acy * adx;

    // scalar triple product AB · (AC x AD)
    double triple = abx * cxp + aby * cyp + abz * czp;

    // signed volume
    return triple / 6.0;
}

// --------------- COUNT INVERTED / DEGENERATE ------------
//volume = 0: good tet, consistent orientation.
//volume < 0: inverted tet.
//volume ≈ 0: degenerate (flat or collapsed).
void count_inverted_and_degenerate() {  
    int num_tets = static_cast<int>(T.size());  // mum_tets = total_tets
    int inverted = 0;
    int degenerate = 0;
    // loops all tets vloume signedVolume
    for (int tid = 0; tid < num_tets; ++tid) {
        double vol = signedVolume(tid);
    //checks for inverted tets
        if (vol < -epsVol) {
            inverted++;
    //check for degenerted tets
        } else if (std::abs(vol) <= epsVol) {
            degenerate++;
        }
    }
// we are using vloume + tolerance to classify and cout tets 
    std::cout << "Volume stats:\n";
    std::cout << "  Total tets:      " << num_tets << "\n";
    std::cout << "  Inverted tets:   " << inverted << "\n";
    std::cout << "  Degenerate tets: " << degenerate << "\n";
}
//------------------ Duplicate tet detection--------------
void detect_duplicate_tets() {
    using Key = std::array<int,4>; // Key is alias and represent tets ID

    std::map<Key, int> counts;  // count how many times this tets appears
    const int num_tets = static_cast<int>(T.size());

    for (int tid = 0; tid < num_tets; ++tid) { //loop tets
        const Tet &tet = T[tid];

        Key key = { tet.v[0], tet.v[1], tet.v[2], tet.v[3] }; //buliding key from the 4 V_id of tets
        std::sort(key.begin(), key.end());  // sorts the 4 V_id inside key in asce order

        counts[key]++;
    }

    int num_duplicate_groups = 0;  // how many differnt tets patterns are duplicat
    int num_duplicate_tets   = 0;  // same tets - 1st occurences 

    for (const auto &kv : counts) {
        const Key &key = kv.first;
        int c = kv.second;
        if (c > 1) {          // c == 1 no dulpicate / c>1 there are dulpicate
            num_duplicate_groups++;
            num_duplicate_tets += (c - 1);

            std::cout << "Duplicate tet (appears " << c << " times): "
                      << key[0] << " " << key[1] << " "
                      << key[2] << " " << key[3] << "\n";
        }
    }

    std::cout << "Duplicate tet summary:\n";
    std::cout << "  Distinct duplicate patterns: " << num_duplicate_groups << "\n";
    std::cout << "  Extra duplicate tets:        " << num_duplicate_tets   << "\n";
}

//-------------Face manifold check: Build all faces (4 per tet)/
//Canonical face key = sorted triple
//Count how many tets incident to each face
//Report faces with count > 2 (non-manifold)//

void face_manifold_check() {
    using FaceKey = std::array<int,3>; //  alias FaceKey to array

    std::map<FaceKey, int> face_counts;  // int - how many tets share the for theta face 

    const int num_tets = static_cast<int>(T.size()); 

    // Helper lambda to insert one face 
    auto add_face = [&](int a, int b, int c) {
        FaceKey key = { a, b, c };
        std::sort(key.begin(), key.end());   // sorts the 3 Id (a,b,c)(b,a,c)(c,b,a) in order
        face_counts[key]++;                  // one more tet uses this face
    };

    // 1) Build all faces
    for (int tid = 0; tid < num_tets; ++tid) {
        const Tet &tet = T[tid];
        int a = tet.v[0];
        int b = tet.v[1];
        int c = tet.v[2];
        int d = tet.v[3];

        //4 triangular Faces of a tetrahedron (unordered triples)
        add_face(a, b, c);
        add_face(a, b, d);
        add_face(a, c, d);
        add_face(b, c, d);
    }
   // interior face: exactly 2 tets
   // boundary face: exactly 1 tets 
    // 2) Report non‑manifold faces (incident to more than 2 tets)
    int non_manifold_faces = 0;

    for (const auto &kv : face_counts) {
        const FaceKey &key = kv.first;
        int count = kv.second;

        if (count > 2) {
            non_manifold_faces++;
            std::cout << "Non-manifold face (used by " << count << " tets): "
                      << key[0] << " " << key[1] << " " << key[2] << "\n";
        }
    }

    std::cout << "Face manifold check:\n";
    std::cout << "  Total unique faces:   " << face_counts.size() << "\n";
    std::cout << "  Non-manifold faces:   " << non_manifold_faces << "\n";
}


//---------------------- end ------------

// --------------- MAIN ------------
int main() {
    std::ifstream in("meshtetsexpanded.txt");
    if (!in) {
        std::cerr << "Error: could not open meshtetsexpanded.txt\n";
        return 1;
    }

    std::string line;
    int line_number = 0;

    while (std::getline(in, line)) {
        ++line_number;
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        std::istringstream iss(line);
        double x[4], y[4], z[4];

        if (!(iss >> x[0] >> y[0] >> z[0]
                  >> x[1] >> y[1] >> z[1]
                  >> x[2] >> y[2] >> z[2]
                  >> x[3] >> y[3] >> z[3])) {
            std::cerr << "Warning: could not parse numeric data on line "
                      << line_number << "\n";
            continue;
        }

        Tet tet;
        for (int i = 0; i < 4; ++i) {
            tet.v[i] = getVertexID(x[i], y[i], z[i]);
        }
        T.push_back(tet);
    }

    in.close();

   // std::cout << "Number of unique vertices: " << Vx.size() << "\n";
   // std::cout << "Number of tets:            " << T.size() << "\n";

    // if (!T.empty()) {
    //     const Tet &t0 = T[0];
    //     std::cout << "First tet vertex IDs: "
    //               << t0.v[0] << " "
    //               << t0.v[1] << " "
    //               << t0.v[2] << " "
    //               << t0.v[3] << "\n";
    // }

    // Index validation
    bool index_ok = validate_index_ranges(true);
    if (!index_ok) {
        std::cerr << "Mesh validation failed (index checks).\n";
        // You can choose to return here or still compute volumes.
        // return 2;
    }

    // Volume-based checks
    count_inverted_and_degenerate();
    
    // Duplicate tet detection
    detect_duplicate_tets();
    
    //Face manifold check:
   
   
    face_manifold_check();


    return 0;
}
