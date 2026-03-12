#include "reconstruction.hpp"
#include "mesh_io.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

static const double coordTol = 1e-8;

void reconstructionTest(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Error: Could not open file for reconstruction test: " << filename << "\n";
        return;
    }

    double maxError = 0.0;
    double sumSquaredError = 0.0;
    long long totalVerticesChecked = 0;
    int failedTets = 0;

    std::string line;
    int line_number = 0;

    while (std::getline(in, line)) {
        ++line_number;
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        double x[4], y[4], z[4];

        if (!(iss >> x[0] >> y[0] >> z[0]
                  >> x[1] >> y[1] >> z[1]
                  >> x[2] >> y[2] >> z[2]
                  >> x[3] >> y[3] >> z[3])) {
            std::cerr << "Warning: parse error on line " << line_number << "\n";
            continue;
        }

        bool tetFailed = false;

        for (int i = 0; i < 4; ++i) {
            long long qx = (long long)std::llround(x[i] / epsV);
            long long qy = (long long)std::llround(y[i] / epsV);
            long long qz = (long long)std::llround(z[i] / epsV);

            Key key(qx, qy, qz);
            auto it = key_to_vertex_id.find(key);

            if (it == key_to_vertex_id.end()) {
                tetFailed = true;
                continue;
            }

            int id = it->second;

            double rx = Vx[id];
            double ry = Vy[id];
            double rz = Vz[id];

            double ex = std::fabs(x[i] - rx);
            double ey = std::fabs(y[i] - ry);
            double ez = std::fabs(z[i] - rz);

            double vertexError = std::max(ex, std::max(ey, ez));
            maxError = std::max(maxError, vertexError);

            sumSquaredError += vertexError * vertexError;
            totalVerticesChecked++;

            if (vertexError > coordTol) tetFailed = true;
        }

        if (tetFailed) failedTets++;
    }

    double rmsError = (totalVerticesChecked > 0)
        ? std::sqrt(sumSquaredError / totalVerticesChecked)
        : 0.0;

    std::cout << "\n=============== Reconstruction Test Summary ===============\n";
    std::cout << "Max Error       : " << maxError << "\n";
    std::cout << "RMS Error       : " << rmsError << "\n";
    std::cout << "Failed Tets     : " << failedTets << "\n";
    std::cout << "Vertices tested : " << totalVerticesChecked << "\n";
    std::cout << "==========================================================\n";
}