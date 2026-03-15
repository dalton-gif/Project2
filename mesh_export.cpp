/*
--------------------------------------------------------------------------
Main purpose of this file is export the data stored in form of Vertices
and Tetrahedron into .msh file supported in a Gmsh software which can plot
the whole mesh

Mesh Class is defined here to store vertices co-ordinates and tetrahedron
connectivity of the mesh.
--------------------------------------------------------------------------
Vectors are used because they can grow dynamically
--------------------------------------------------------------------------
*/

#include<iostream>
#include<vector>
#include<fstream>
#include<string>
#include<cstdint>

#include "mesh_io.hpp"
#include "mesh_export.hpp"

using namespace std;


/*
Function : exportMSH
Purppose : Export the data into .msh file for visualisation
*/

void Mesh::exportMSH(const string& filename)
{
    ofstream out(filename);                    // Create output file stream

    if(!out)
    {
        cout<<"Error opening output file\n"; // Checking file open to avoid error handling
                return;
    }


   int nV = x.size();              // Numver of Vertices
   int nT = t0.size();             // NNumer of Tetrahedra

   /* Format or Structure of .msh file
   
        $MeshFormat                   // Header with the version Format
        2.2 0 8
        $EndMeshFormat

        $Nodes
        #nV (Number of Vertices/Nodes)
        1 x1 y1 z1                     // Indicing starts from 1 not 0
        2 x2 y2 z2                     // IndexID X coord Y coord Z coord
        3 x3 y3 z3
        4 x4 y4 z4
        ...
        ...
        $EndNodes

        $Elements
        #n (Number of elements; here Tets)
        1 4 0 t0 t1 t2 t3            // ele.ID ele.Type ele.Tag Node1 Node 2 Node 3 Node 4        
        ...                          //   1 -> ele.ID
        ...                          //   4 -> ele.Type - 4 for Tetrahedron                    
        ...                          //   0 -> ele.Tag - 0 (no tags used in our case)
        ...                          //   t0 t1 t2 t3 -> Nodes
        $EndElements
   */

   //Writing the Mesh Header file in Standard Format for Gmsh software

    out<<"$MeshFormat\n";
    out<<"2.2 0 8\n";
    out<<"$EndMeshFormat\n";

    // Writing Vertices in Nodes format inside .msh file 
    out<<"$Nodes\n";
    out<<nV<< "\n";

    for(int i=0;i<nV;i++)
    {
        out << i+1   << " " << x[i] << " " 
            << y[i] << " " << z[i] << "\n";
    }

    out << "$EndNodes\n";

    // Writing Tetrahedrons in Elements format inside .msh file 

    out<<"$Elements\n";
    out<<nT << "\n";

    for(int i=0;i<nT;i++)
    {
        out << i+1 <<" "<< 4 << " " << 0 << " "
            << t0[i]+1 << " "<< t1[i]+1 << " " 
            << t2[i]+1 << " " <<t3[i]+1 <<"\n";

    }
    out <<"$EndElements\n";

    out.close();                // Close the file

    cout <<"Mesh Exported Successfullly to : " << filename << endl;
}




