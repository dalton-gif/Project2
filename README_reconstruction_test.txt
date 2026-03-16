reconstruction_test.cpp – README

Overview This file implements a mesh reconstruction verification test.
The purpose of this script is to validate that the mesh constructed in
the main program matches the original tetrahedral mesh data from the raw
input text file.

The function re-reads the original expanded mesh file, reconstructs
vertex identifiers using the same quantization and hashing method used
during mesh construction, and compares the stored vertex coordinates
with the original coordinates.

The program reports reconstruction accuracy statistics including maximum
error, RMS error, and the number of tetrahedra that fail the
verification test.

------------------------------------------------------------------------

Main Function

reconstructionTest(const std::string& filename)

This function verifies whether the mesh reconstruction process preserved
the original vertex coordinates within a defined tolerance.

Input: filename → name of the expanded mesh text file containing
tetrahedron vertex coordinates.

Each line in the file contains 12 values representing four vertices:

x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4

------------------------------------------------------------------------

Working Principle

Step 1 – Read the Expanded Mesh File The program reads the input file
line by line using: - std::ifstream - std::getline - std::istringstream

Each line represents one tetrahedron containing four vertices. The
coordinates are stored temporarily as arrays:

x[4], y[4], z[4]

------------------------------------------------------------------------

Step 2 – Quantize Vertex Coordinates

To reproduce the same vertex identifiers created during mesh
construction, the coordinates are quantized.

Quantization converts floating-point coordinates into integer grid
values using a tolerance value.

Formula used:

qx = round(x / epsV) qy = round(y / epsV) qz = round(z / epsV)

Where epsV is the vertex tolerance used during mesh construction.

The quantized coordinates are stored as a tuple:

Key = (qx, qy, qz)

------------------------------------------------------------------------

Step 3 – Lookup Vertex ID Using Hash Table

The quantized key is searched in a hash map:

unordered_map<Key, int>

This map was created during mesh construction and stores:

Quantized Vertex → Vertex ID

If the key exists, the corresponding vertex ID is retrieved. If the key
does not exist, the tetrahedron is marked as failed.

------------------------------------------------------------------------

Step 4 – Retrieve Stored Vertex Coordinates

Using the retrieved vertex ID, the stored coordinates are accessed from
the global vertex arrays:

Vx[id] Vy[id] Vz[id]

These arrays store the unique vertices generated during reconstruction.

------------------------------------------------------------------------

Step 5 – Compare Original and Reconstructed Coordinates

The program compares the original vertex coordinates with the
reconstructed ones.

Errors are computed as:

ex = |x_original − x_reconstructed| ey = |y_original − y_reconstructed|
ez = |z_original − z_reconstructed|

The vertex error is defined as sqrt(sum squared error)

vertexError = max(ex, ey, ez)

------------------------------------------------------------------------

Step 6 – Compute Reconstruction Statistics

During verification the program tracks:

Maximum Error The largest coordinate difference encountered.

RMS Error Root Mean Square error across all tested vertices.

RMS = sqrt(sum(error^2) / totalVertices)

Failed Tetrahedra A tetrahedron is marked as failed if: - a vertex
cannot be found in the hash table - coordinate error exceeds the
tolerance

Tolerance used: coordTol = 1e-8

------------------------------------------------------------------------

Output

After processing the file the program prints a summary:

=============== Reconstruction Test Summary ===============

Max Error : value RMS Error : value Failed Tets : count Vertices Tested
: count

==========================================================

------------------------------------------------------------------------

Data Structures Used

Vertex Coordinate Storage

std::vector Vx std::vector Vy std::vector Vz

Each index represents a vertex ID.

------------------------------------------------------------------------

Quantized Vertex Key

typedef std::tuple<long long, long long, long long> Key

Stores integerized coordinates used for vertex lookup.

------------------------------------------------------------------------

Hash Map

std::unordered_map<Key, int>

Maps: Quantized Vertex → Vertex ID

Custom hash and equality functions are implemented to allow tuples to be
used as keys.

------------------------------------------------------------------------

C++ Concepts Used

File Input/Output - std::ifstream - std::getline

String Parsing - std::istringstream

STL Containers - std::vector - std::unordered_map

Tuples - std::tuple

Custom Hash Functions Used to allow tuple keys in unordered_map.

Mathematical Functions from  - std::fabs - std::llround - std::sqrt

------------------------------------------------------------------------

External Variables Used

epsV Vertex tolerance used for quantization.

Vx, Vy, Vz Arrays storing unique vertex coordinates.

key_to_vertex_id Hash map mapping quantized vertices to vertex IDs.

------------------------------------------------------------------------

Purpose in the Overall Project

This file implements the mesh reconstruction verification step. It
ensures that:

-   Vertex deduplication worked correctly
-   Quantization did not introduce large numerical errors
-   The reconstructed mesh accurately represents the original mesh.
