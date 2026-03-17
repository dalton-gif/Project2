mesh.cpp – README

Overview :

This file defines the Mesh class used to store mesh data and
export it into a .msh file format that can be visualized using Gmsh
software.

The main purpose of this file is to take the mesh data (vertices and
tetrahedron connectivity) stored in memory and write it into a standard
Gmsh mesh file. This allows the reconstructed mesh to be visualized and
verified using external visualization tools.

------------------------------------------------------------------------

Mesh Class

The Mesh class stores the geometry and connectivity of a tetrahedral
mesh.

It contains:

1.  Vertex coordinate storage
2.  Tetrahedron connectivity
3.  A function to export the mesh into a .msh file

------------------------------------------------------------------------

Vertex Storage

The mesh stores vertex coordinates using three vectors:

vector x vector y vector z

Each index represents one vertex in the mesh.

Example: Vertex i has coordinates:

x[i], y[i], z[i]

Vectors are used because they allow dynamic resizing as new vertices are
added to the mesh.

------------------------------------------------------------------------

Tetrahedron Connectivity

Tetrahedra are stored using four vectors:

vector t0 vector t1 vector t2 vector t3

Each index represents one tetrahedron.

Example: Tet i is defined by four vertex indices:

t0[i], t1[i], t2[i], t3[i]

These indices refer to vertices stored in the vertex coordinate arrays.

The type uint32_t (32‑bit unsigned integer) is used because vertex
indices are always non‑negative and this type is memory efficient.

------------------------------------------------------------------------

Main Function

exportMSH(const string& filename)

Purpose: This function exports the mesh data into a .msh file that can
be opened in Gmsh for visualization.

Input: filename → name of the output .msh file.

Example: mesh.exportMSH(“output.msh”)

------------------------------------------------------------------------

Steps Performed in exportMSH()

Step 1 – Open Output File

The function creates an output file stream using:

ofstream out(filename)

If the file cannot be opened, the program prints an error message and
exits the function.

------------------------------------------------------------------------

Step 2 – Determine Mesh Size

The number of vertices and tetrahedra are determined using vector sizes.

nV = x.size() → number of vertices nT = t0.size() → number of tetrahedra

------------------------------------------------------------------------

Step 3 – Write Gmsh Mesh Header

The function writes the standard mesh header required by Gmsh.

MeshFormat
2.208
EndMeshFormat

This specifies that the mesh file uses version 2.2 of the Gmsh format.

------------------------------------------------------------------------

Step 4 – Write Vertex Data (Nodes Section)

Vertices are written under the $Nodes section.

Format:

$Nodes
#Nodes
NodeID x y z
...
EndNodes

Important detail: Node indexing in Gmsh starts from 1 instead of 0.

Therefore the code writes:

i + 1

as the node ID.

------------------------------------------------------------------------

Step 5 – Write Tetrahedron Elements

Tetrahedra are written under the $Elements section.

Format:

$Elements
#Elements
ElementID ElementType ElementTag Node1 Node2 Node3 Node4 
...
EndElements

For tetrahedra:

elementType = 4

Tags are set to zero because no additional element properties are used.

Example line written by the program:

1 4 0 n1 n2 n3 n4

------------------------------------------------------------------------

Index Conversion

Since C++ arrays start at index 0 but Gmsh starts at index 1, the code
converts vertex indices using:

t0[i] + 1 t1[i] + 1 t2[i] + 1 t3[i] + 1

This ensures correct node numbering in the output mesh file.

------------------------------------------------------------------------

Output

After writing all mesh data, the file is closed using:

out.close()

The program prints a confirmation message:

Mesh Exported Successfully to : filename

------------------------------------------------------------------------

C++ Concepts Used

Classes The Mesh class encapsulates mesh data and functionality.

STL Containers vector is used for dynamic storage of vertices and
tetrahedra.

File Output ofstream is used to write the mesh file.

Standard Library Types uint32_t is used for efficient storage of vertex
indices.

Control Structures Loops are used to iterate through vertices and
tetrahedra.

------------------------------------------------------------------------

Purpose in the Overall Project

This file is responsible for the final step of the mesh reconstruction
pipeline: exporting the processed mesh into a format that can be
visualized.

By writing the mesh into the Gmsh .msh format, the reconstructed mesh
can be opened and inspected using the Gmsh software.
