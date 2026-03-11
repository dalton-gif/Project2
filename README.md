# Project2
MEEN 704 - Mesh Project

**Style Guide**



**Tolerance** //
double epsV      = 1e-10;
double epsVol    = 1e-12;
double coordTol  = 1e-12;
Canonical keys: face = sorted(3), tet = sorted(4)

Below is a complete updated README in plain text format. You can copy–paste it into a file named `README.txt`.

***

README — Tetrahedral Mesh Vertex Quantization and Storage
=========================================================

Overview
--------

This C++ program processes a mesh file containing tetrahedra (each defined by 4 vertices), removes duplicate vertices through coordinate quantization, and builds an efficient in-memory representation of unique vertices and tetrahedra. It then allows the user to query a tetrahedron by ID, see its vertex IDs, and then query a vertex by ID to see its coordinates.

The code demonstrates techniques such as:
- Quantizing 3D coordinates to enforce numeric tolerance.
- Using a hash map to track unique 3D points.
- Storing vertices in a Structure of Arrays (SoA) layout for memory efficiency.
- Simple console interaction to inspect the mesh.

Input File Format
-----------------

The program expects an input file named:

    mesh_tets_expanded.txt

Each line (non-comment, non-empty) contains 12 floating-point numbers, representing the coordinates of 4 vertices:

    x0 y0 z0 x1 y1 z1 x2 y2 z2 x3 y3 z3

Lines beginning with `#` are ignored as comments. Empty lines are skipped.

Key Concepts
------------

1. Quantization

Coordinates are quantized using a small epsilon tolerance `epsV` to merge nearly identical vertices. Two vertex positions differing by less than this tolerance will be treated as the same point.

This is implemented as:

- Scale each coordinate by `1 / epsV`.
- Round using `std::llround` to get an integer representation.

2. Hashing and Equality

Each triplet of quantized coordinates `(qx, qy, qz)` is stored in a tuple type `Key`:

    typedef std::tuple<long long, long long, long long> Key;

This key is used to:
- Identify if a vertex already exists (via an unordered map).
- Retrieve or assign a unique vertex ID.

Custom hash (`KeyHash`) and equality (`KeyEq`) functors are defined so that `Key` can be used as a key type in the unordered map.

Global Structures and Constants
-------------------------------

- `const double epsV = 1e-10;`

  Vertex tolerance used for quantization.

- `std::vector<double> Vx, Vy, Vz;`

  Parallel arrays holding x, y, and z coordinates of all unique vertices. The index in these vectors is the vertex ID.

- `struct Tet { int v[4]; };`

  Represents a tetrahedron using 4 integer vertex IDs.

- `std::vector<Tet> T;`

  Stores all tetrahedra in the mesh.

- `std::unordered_map<Key, int, KeyHash, KeyEq> key_to_vertex_id;`

  Maps quantized coordinate keys to a corresponding vertex ID.

Function: getVertexID
---------------------

Signature:

    int getVertexID(double x, double y, double z);

Purpose:

- Ensure that each unique spatial position gets a single vertex ID.
- Reuse existing IDs for vertices at the same (quantized) coordinates.

Steps:

1. Quantize the input coordinates `(x, y, z)`:

   - Compute `qx = llround(x / epsV)`.
   - Compute `qy = llround(y / epsV)`.
   - Compute `qz = llround(z / epsV)`.

2. Create a `Key` from `(qx, qy, qz)`.

3. Look for this key in `key_to_vertex_id`:
   - If found, return the existing vertex ID.
   - If not found:
     - Create a new vertex ID equal to `Vx.size()`.
     - Push `x` to `Vx`, `y` to `Vy`, and `z` to `Vz`.
     - Store the mapping `key -> new_id` in `key_to_vertex_id`.
     - Return the new vertex ID.

Main Program Flow
-----------------

1. Open the input file:

   - Attempts to open `mesh_tets_expanded.txt` using an input file stream.
   - If the file cannot be opened, prints an error and exits with a non-zero status.

2. Read the file line by line:

   - Uses `std::getline` to read each line.
   - Maintains `line_number` to track which line is being processed.
   - Skips:
     - Empty lines.
     - Comment lines starting with `#`.

3. Parse vertex coordinates for each tetrahedron:

   - For each valid data line, uses an `std::istringstream` to extract 12 doubles:
     - `x[0], y[0], z[0]`
     - `x[1], y[1], z[1]`
     - `x[2], y[2], z[2]`
     - `x[3], y[3], z[3]`
   - If the extraction fails, prints a warning including the line number and continues to the next line.

4. Build tetrahedra and vertices:

   - For each of the 4 vertices in the line:
     - Calls `getVertexID(x[i], y[i], z[i])` to obtain a unique vertex ID.
     - Stores this ID into the `Tet tet;` structure.
   - Pushes the completed `tet` into the vector `T`.

5. Summary output:

   - After reading all lines:
     - Prints the number of unique vertices: `Vx.size()`.
     - Prints the number of tetrahedra: `T.size()`.

User Interaction Section
------------------------

After reading and storing all tetrahedra and vertices, the program allows the user to query specific elements.

The behavior is:

1. Check that at least one tetrahedron exists:

   - If `T` is empty, no user interaction is performed (the program just exits after printing counts).

2. Prompt for a tetrahedron ID:

   - Displays a message such as:

         Enter a tet ID (0 to N-1):

     where `N` is `T.size()`.

   - Reads an integer `tet_id` from standard input.

   - If `tet_id` is outside the valid range `[0, T.size() - 1]`, the program prints:

         Invalid tet ID.

     and does not proceed to vertex ID queries.

3. Print the chosen tetrahedron’s vertex IDs:

   - If `tet_id` is valid, the program prints:

         Tet <tet_id> vertex IDs: v0 v1 v2 v3

     where `v0`, `v1`, `v2`, and `v3` are the four vertex indices of `T[tet_id]`.

4. Prompt for a vertex ID:

   - Displays a message such as:

         Enter a vertex ID (0 to M-1):

     where `M` is `Vx.size()`.

   - Reads an integer `vid` from standard input.

   - If `vid` is outside the valid range `[0, Vx.size() - 1]`, the program prints:

         Invalid vertex ID.

     and does not print any coordinates.

5. Print the chosen vertex’s coordinates:

   - If `vid` is valid, the program prints:

         Vertex <vid> coords: x y z

     where `x = Vx[vid]`, `y = Vy[vid]`, and `z = Vz[vid]`.

Example Interaction
-------------------

Assume the mesh file yields 25 unique vertices and 10 tetrahedra.

Example console session:

    Number of unique vertices: 25
    Number of tets: 10
    Enter a tet ID (0 to 9): 3
    Tet 3 vertex IDs: 4 7 12 15
    Enter a vertex ID (0 to 24): 7
    Vertex 7 coords: 0.5 1.0 0.25

In this example:
- The user queries tetrahedron with index 3, which consists of vertices 4, 7, 12, and 15.
- Then the user queries vertex 7 and sees its coordinates.

Intent of Each Major Section
----------------------------

- Constants and global vectors:
  - Define the tolerance and data containers for vertices and tetrahedra.

- Struct definitions (`Tet`, `Key`, hash and equality functors):
  - Represent geometric objects and provide a way to uniquely identify quantized points.

- `getVertexID` function:
  - Encapsulates the logic for deduplicating vertices and assigning consistent IDs.

- File-reading loop in `main`:
  - Reads, validates, and parses tetrahedron data from the input file.
  - Uses `getVertexID` to build an efficient vertex and tetrahedron structure.

- Summary and interactive query section:
  - Gives a quick overview of mesh size.
  - Lets the user inspect a tetrahedron and a specific vertex by ID via console input.


End of README
-------------