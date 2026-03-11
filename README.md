# Project2
MEEN 704 - Mesh Project

**Style Guide**



**Tolerance** //
double epsV      = 1e-10;
double epsVol    = 1e-12;
double coordTol  = 1e-12;
Canonical keys: face = sorted(3), tet = sorted(4)

//------------------------------ Deliverable-B --------------------------------------------------------------//

// ----------------- Mesh.cpp -----------------//

const double epsVol = 1e-12;   // volume tolerance; adjust if needed

// --------------- INDEX VALIDATION ------------
// We need to check that every tet vertex ID is in the range [0, num_vertices-1].
// Example: if num_vertices = 5, valid IDs are 0..4 (pass).
//          IDs like -1 or 5 are invalid (fail).
bool validate_index_ranges(bool verbose = true) {
    bool ok = true; // Start by assuming the mesh is valid.

    // num_vertices = length of the vertex arrays (Vx,Vy,Vz)
    const int num_vertices = static_cast<int>(Vx.size());
    // num_tets = how many tetrahedra we have
    const int num_tets     = static_cast<int>(T.size());

    if (verbose) {
        // Display how many vertices and tets we are checking
        std::cout << "Running index-range validation...\n";
        std::cout << "  num_vertices = " << num_vertices
                  << ", num_tets = " << num_tets << "\n";
    }

    // Loop over all tetrahedra: tid goes from 0 to num_tets-1
    for (int tid = 0; tid < num_tets; ++tid) {
        const Tet &tet = T[tid];  // Reference to the current tet
        // Check each of the 4 vertex IDs in this tet
        for (int loc = 0; loc < 4; ++loc) {
            int vid = tet.v[loc];  // Vertex ID at local position loc

            // Valid range is [0, num_vertices-1]
            if (vid < 0 || vid >= num_vertices) {
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
// Compute signed volume of tet tid using scalar triple product / 6.
// If volume > 0 : good tet, consistent orientation.
// If volume < 0 : inverted tet.
// If volume ≈ 0 : degenerate (flat or collapsed).
double signedVolume(int tid) {
    const Tet &tet = T[tid];

    // Extract the four vertex IDs of this tet
    int ia = tet.v[0];
    int ib = tet.v[1];
    int ic = tet.v[2];
    int id = tet.v[3];

    // Positions: look up each vertex’s coordinates from the arrays Vx, Vy, Vz.
    double ax = Vx[ia], ay = Vy[ia], az = Vz[ia];
    double bx = Vx[ib], by = Vy[ib], bz = Vz[ib];
    double cx = Vx[ic], cy = Vy[ic], cz = Vz[ic];
    double dx = Vx[id], dy = Vy[id], dz = Vz[id];

    // Edge vectors: build edge vectors from A
    double abx = bx - ax, aby = by - ay, abz = bz - az;
    double acx = cx - ax, acy = cy - ay, acz = cz - az;
    double adx = dx - ax, ady = dy - ay, adz = dz - az;

    // cross = AC x AD, (cxp,cyp,czp) is that cross vector
    double cxp = acy * adz - acz * ady;
    double cyp = acz * adx - acx * adz;
    double czp = acx * ady - acy * adx;

    // scalar triple product AB · (AC x AD)
    double triple = abx * cxp + aby * cyp + abz * czp;

    // signed volume
    return triple / 6.0;
}

// --------------- COUNT INVERTED / DEGENERATE ------------
// Classification using volume + tolerance epsVol:
//   vol < -epsVol        -> inverted tet
//   |vol| <= epsVol      -> degenerate tet (near zero / flat)
//   vol >  epsVol        -> normal tet
void count_inverted_and_degenerate() {
    int num_tets = static_cast<int>(T.size());  // num_tets = total number of tets
    int inverted = 0;
    int degenerate = 0;

    // Loop over all tets and compute signedVolume
    for (int tid = 0; tid < num_tets; ++tid) {
        double vol = signedVolume(tid);

        // Check for inverted tets (clearly negative volume)
        if (vol < -epsVol) {
            inverted++;
        // Check for degenerate tets (volume very close to zero)
        } else if (std::abs(vol) <= epsVol) {
            degenerate++;
        }
    }

    // We are using volume + tolerance to classify and count tets
    std::cout << "Volume stats:\n";
    std::cout << "  Total tets:      " << num_tets << "\n";
    std::cout << "  Inverted tets:   " << inverted << "\n";
    std::cout << "  Degenerate tets: " << degenerate << "\n";
}

//------------------ Duplicate tet detection--------------
// Detect tets that have exactly the same 4 vertex IDs (in any order).
void detect_duplicate_tets() {
    using Key = std::array<int,4>; // Key is an alias that represents a tet by 4 vertex IDs

    // counts[key] = how many times this tet pattern appears
    std::map<Key, int> counts;
    const int num_tets = static_cast<int>(T.size());

    // Loop over all tets
    for (int tid = 0; tid < num_tets; ++tid) {
        const Tet &tet = T[tid];

        // Build key from the 4 vertex IDs of the tet
        Key key = { tet.v[0], tet.v[1], tet.v[2], tet.v[3] };
        // Sorts the 4 vertex IDs inside key in ascending order (canonical form)
        std::sort(key.begin(), key.end());

        counts[key]++;
    }

    int num_duplicate_groups = 0;  // how many different tet patterns are duplicated
    int num_duplicate_tets   = 0;  // total duplicates beyond the first occurrence

    for (const auto &kv : counts) {
        const Key &key = kv.first;
        int c = kv.second;
        // c == 1 -> no duplicate, c > 1 -> there are duplicates
        if (c > 1) {
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

//------------- Face manifold check -------------
// Build all faces (4 per tet).
// Canonical face key = sorted triple of vertex IDs.
// Count how many tets are incident to each face.
// Report faces with count > 2 as non-manifold
// (interior face: exactly 2 tets, boundary face: exactly 1 tet).
void face_manifold_check() {
    using FaceKey = std::array<int,3>; // alias FaceKey to array<int,3>

    // face_counts[key] = how many tets share this triangular face
    std::map<FaceKey, int> face_counts;

    const int num_tets = static_cast<int>(T.size());

    // Helper lambda to insert one face
    auto add_face = [&](int a, int b, int c) {
        FaceKey key = { a, b, c };
        // Sorts the 3 IDs (a,b,c) in order, so different permutations map to same key
        std::sort(key.begin(), key.end());
        face_counts[key]++;   // one more tet uses this face
    };

    // 1) Build all faces
    for (int tid = 0; tid < num_tets; ++tid) {
        const Tet &tet = T[tid];
        int a = tet.v[0];
        int b = tet.v[1];
        int c = tet.v[2];
        int d = tet.v[3];

        // 4 triangular faces of a tetrahedron (unordered triples)
        add_face(a, b, c);
        add_face(a, b, d);
        add_face(a, c, d);
        add_face(b, c, d);
    }

    // interior face: exactly 2 tets
    // boundary face: exactly 1 tet
    // non-manifold face: more than 2 tets
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

----------------------- main----------------------------------

bool index_ok = validate_index_ranges(true);
if (!index_ok) {
    std::cerr << "Mesh validation failed (index checks).\n";
    // optionally: return 2;
}

count_inverted_and_degenerate();
detect_duplicate_tets();
face_manifold_check();

