#pragma once

#include<tuple>
#include<unordered_map>

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