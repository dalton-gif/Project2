#include "validation.hpp"
#include "mesh_io.hpp"

#include <iostream>
#include <map>
#include <array>
#include <cmath>
#include <algorithm>

static const double epsVol = 1e-12; // Volume tolerance

//----------------- Index Validation ------------------
bool validate_index_ranges(bool verbose)
{
    bool ok = true;     //Start by assuming the mesh is valid.
    const int num_vertices = static_cast<int>(Vx.size()); //length of your x‑coordinate
    const int num_tets = static_cast<int>(T.size()); //how many tetrahedra we have.

    if(verbose)
    {
        std::cout << "===========[Deliverable-B]=========\n";
        std::cout << "Running Index-Range validation...\n";
        std::cout << "num_vertices = " << num_vertices
                  << "num_tets = " << num_tets <<"\n";

    }

    for(int tid=0; tid < num_tets; ++tid)
    {
        const Tet& tet = T[tid];  // referance to the current tetes
        for (int loc = 0; loc<4; ++loc) // checking in each tete location 
        {
            int vid = tet.v[loc];
            if (vid < 0 || vid >= num_vertices) //"OR"fun valid rage id 0 to n-1
            {
                ok = false;
                std::cerr <<"Index Error: tet" << tid
                          <<",local vertex"<< loc
                          <<"has ID"<< vid
                          <<"outside [0,"<<(num_vertices - 1)<<"]\n";
            }
        }
    }
    if (verbose)
    {
        if(ok)
        {
            std::cout << "Index-range Validation PASSED.\n";
        }
        else
        {
            std::cout <<"Index-range Validation FAILED.\n";
        }
    }
    return ok;
}
//----------------------SIGNED TET VOLUME ----------------------
double signedVolume(int tid)
{
    const Tet& tet = T[tid]; //Extracts the four vertex IDs of this tet:
    int ia = tet.v[0];
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
    return triple/ 6.0;

}
//---------------- COUNT INVERTED / DEGENERATE ----------------
//volume = 0: good tet, consistent orientation.
//volume < 0: inverted tet.
//volume ≈ 0: degenerate (flat or collapsed).
void count_inverted_and_degenerate()
{
    int num_tets = static_cast<int>(T.size()); // num_tets = total_tets
    int inverted = 0;
    int degenerate = 0;

    // loops all tets vloume signedVolume
    for (int tid = 0; tid < num_tets; ++tid)
    {
        double vol = signedVolume(tid);
        if (vol < -epsVol)  //checks for inverted tets
        {
            inverted++;
        }
        else if (std::abs(vol)<= epsVol) //check for degenerted tets
        {
            degenerate++;
        }
    }
    // we are using vloume + tolerance to classify and cout tets 
    std::cout << "vloume stats:\n";
    std::cout << " Total tets:    "<< num_tets <<"\n";
    std::cout << " Inverted tets: "<< inverted <<"\n";
    std::cout << " Degenerate tets:"<< degenerate <<"\n";
}
//------------- Duplicate tet detection -------------
void detect_duplicate_tets()
{
    using Key = std::array<int,4>;  // Key is alias and represent tets ID
    std::map<Key, int> counts;      // count how many times this tets appears
    const int num_tets = static_cast<int>(T.size());

    for (int tid = 0; tid < num_tets; ++tid)  //loop tets
    {
        const Tet &tet =T[tid];
        Key key ={tet.v[0],tet.v[1],tet.v[2],tet.v[3]};  //buliding key from the 4 V_id of tets
        std::sort(key.begin(), key.end());       // sorts the 4 V_id inside key in asce order

        counts[key]++;
    }

    int num_duplicate_group = 0;   // how many differnt tets patterns are duplicat
    int num_duplicate_tets =0;   // same tets - 1st occurences 

    for (const auto &kv : counts)
    {
        const Key &key = kv.first;
        int c = kv.second;
        if (c >1)                 // c == 1 no dulpicate / c>1 there are dulpicate
        {
            num_duplicate_group++;
            num_duplicate_tets +=(c - 1);

            std::cout <<" Duplicate tet(appears " << c <<"time):"
                      <<key[0]<<" " << key[1]<<" "
                      <<key[2]<<" " << key[3]<<"\n";
        }
    }
    std::cout <<"Duplicate tet summary:\n";
    std::cout <<" Distinct duplicate patterns:" << num_duplicate_group <<"\n";
    std::cout <<" Extra duplicate tets:       " << num_duplicate_tets <<"\n";
}
//---------Face manifold check: Build all faces (4 per tet)/-------------
//Canonical face key = sorted triple
//Count how many tets incident to each face
//Report faces with count > 2 (non-manifold)//
void face_manifold_check(){
    using FaceKey = std::array<int,3>;   //  alias FaceKey to array
    std::map<FaceKey, int> face_counts;       // int - how many tets share the for theta face
    const int num_tets = static_cast<int>(T.size());

    auto add_face =[&](int a, int b, int c)
    {
      FaceKey key = {a,b,c};
      std::sort(key.begin(),key.end());   // sorts the 3 Id (a,b,c)(b,a,c)(c,b,a) in order
      face_counts[key]++;     // one more tet uses this face
    };

    for (int tid = 0; tid < num_tets; ++tid)
    {
        const Tet &tet = T[tid];  //1) Build all faces
        int a = tet.v[0];
        int b = tet.v[1];
        int c = tet.v[2];
        int d = tet.v[3];

        add_face(a, b, c);  //4 triangular Faces of a tetrahedron (unordered triples)
        add_face(a, b, d);
        add_face(a, c, d);
        add_face(b, c, d);
    }
     
    int non_manifold_faces =0;
    for (const auto &kv : face_counts)
    {
       const FaceKey &key =kv.first;
       int count = kv.second;
       
       if (count >2)
       {
        non_manifold_faces++;
        std::cout << "NoN-manifold face (used by " << count << "tets):"
                  << key[0] << " " <<key[1] << " " << key[2] << "\n";
       }
    }

    std::cout << "Face manifold check:\n";
    std::cout << " Total unique faces:   "<< face_counts.size() << "\n";
    std::cout << " Non-manifold faces:   "<< non_manifold_faces << "\n";

}
