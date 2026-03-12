#include "validation.hpp"
#include "mesh_io.hpp"

#include <iostream>
#include <map>
#include <array>
#include <algorithm>
#include <cmath>

static const double epsVol = 1e-12; // volume tolerance

bool validate_index_ranges(bool verbose) {
    bool ok = true;
    const int num_vertices = static_cast<int>(Vx.size());
    const int num_tets     = static_cast<int>(T.size());

    if (verbose) {
        std::cout << "Running index-range validation...\n";
        std::cout << "  num_vertices = " << num_vertices
                  << ", num_tets = " << num_tets << "\n";
    }

    for (int tid = 0; tid < num_tets; ++tid) {
        const Tet& tet = T[tid];
        for (int loc = 0; loc < 4; ++loc) {
            int vid = tet.v[loc];
            if (vid < 0 || vid >= num_vertices) {
                ok = false;
                std::cerr << "Index error: tet " << tid
                          << ", local vertex " << loc
                          << " has ID " << vid
                          << " outside [0," << (num_vertices - 1) << "]\n";
            }
        }
    }

    if (verbose) std::cout << (ok ? "Index-range validation PASSED.\n"
                                 : "Index-range validation FAILED.\n");
    return ok;
}

double signedVolume(int tid) {
    const Tet& tet = T[tid];
    int ia = tet.v[0], ib = tet.v[1], ic = tet.v[2], id = tet.v[3];

    double ax = Vx[ia], ay = Vy[ia], az = Vz[ia];
    double bx = Vx[ib], by = Vy[ib], bz = Vz[ib];
    double cx = Vx[ic], cy = Vy[ic], cz = Vz[ic];
    double dx = Vx[id], dy = Vy[id], dz = Vz[id];

    double abx = bx - ax, aby = by - ay, abz = bz - az;
    double acx = cx - ax, acy = cy - ay, acz = cz - az;
    double adx = dx - ax, ady = dy - ay, adz = dz - az;

    double cxp = acy * adz - acz * ady;
    double cyp = acz * adx - acx * adz;
    double czp = acx * ady - acy * adx;

    double triple = abx * cxp + aby * cyp + abz * czp;
    return triple / 6.0;
}

void count_inverted_and_degenerate() {
    int num_tets = static_cast<int>(T.size());
    int inverted = 0;
    int degenerate = 0;

    for (int tid = 0; tid < num_tets; ++tid) {
        double vol = signedVolume(tid);
        if (vol < -epsVol) inverted++;
        else if (std::abs(vol) <= epsVol) degenerate++;
    }

    std::cout << "Volume stats:\n";
    std::cout << "  Total tets:      " << num_tets << "\n";
    std::cout << "  Inverted tets:   " << inverted << "\n";
    std::cout << "  Degenerate tets: " << degenerate << "\n";
}

void detect_duplicate_tets() {
    using TetKey = std::array<int,4>;
    std::map<TetKey, int> counts;

    for (int tid = 0; tid < (int)T.size(); ++tid) {
        TetKey key = {T[tid].v[0], T[tid].v[1], T[tid].v[2], T[tid].v[3]};
        std::sort(key.begin(), key.end());
        counts[key]++;
    }

    int groups = 0, extras = 0;
    for (const auto& kv : counts) {
        if (kv.second > 1) {
            groups++;
            extras += (kv.second - 1);
            const auto& k = kv.first;
            std::cout << "Duplicate tet (appears " << kv.second << " times): "
                      << k[0] << " " << k[1] << " " << k[2] << " " << k[3] << "\n";
        }
    }

    std::cout << "Duplicate tet summary:\n";
    std::cout << "  Distinct duplicate patterns: " << groups << "\n";
    std::cout << "  Extra duplicate tets:        " << extras << "\n";
}

void face_manifold_check() {
    using FaceKey = std::array<int,3>;
    std::map<FaceKey, int> face_counts;

    auto add_face = [&](int a, int b, int c) {
        FaceKey key = {a,b,c};
        std::sort(key.begin(), key.end());
        face_counts[key]++;
    };

    for (int tid = 0; tid < (int)T.size(); ++tid) {
        int a = T[tid].v[0], b = T[tid].v[1], c = T[tid].v[2], d = T[tid].v[3];
        add_face(a,b,c);
        add_face(a,b,d);
        add_face(a,c,d);
        add_face(b,c,d);
    }

    int non_manifold = 0;
    for (const auto& kv : face_counts) {
        if (kv.second > 2) {
            non_manifold++;
            const auto& f = kv.first;
            std::cout << "Non-manifold face (used by " << kv.second << " tets): "
                      << f[0] << " " << f[1] << " " << f[2] << "\n";
        }
    }

    std::cout << "Face manifold check:\n";
    std::cout << "  Total unique faces: " << face_counts.size() << "\n";
    std::cout << "  Non-manifold faces: " << non_manifold << "\n";
}