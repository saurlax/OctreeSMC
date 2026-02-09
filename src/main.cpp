#include <iostream>

#include "ToolMesh.h"
#include "OctreeSMC.h"

using namespace std;
using namespace MeshLib;

void main(int argc, char **argv)
{
	CTMesh mesh;
	mesh.read_m("amphora.m");
	COctreeSMC<CTMesh> smc(&mesh);
	CTMesh *quad = smc.quad_mesh();
	quad->write_m("amphora_quad.m");
	cout << "Finished!" << endl;
}