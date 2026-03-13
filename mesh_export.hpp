#pragma once
#include <vector>
#include <string>
#include <cstdint>

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

    void exportMSH(const std::string& filename);
};