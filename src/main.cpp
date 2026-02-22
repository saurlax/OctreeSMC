#include <iostream>

#include "ToolMesh.h"
#include "OctreeSMC.h"

using namespace std;
using namespace MeshLib;

void main(int argc, char **argv)
{
	CTMesh mesh;
	mesh.read_m("eight.m");
	COctreeSMC<CTMesh> smc(&mesh);
	CTMesh *quad = smc.quad_mesh();
	quad->write_m("eight_quad.m");
	cout << "Finished!" << endl;
}