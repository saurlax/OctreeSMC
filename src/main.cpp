#include <iostream>

#include "ToolMesh.h"
#include "OctreeSMC.h"

using namespace std;
using namespace MeshLib;

void main(int argc, char **argv)
{
	CTMesh mesh;
	mesh.read_obj("pcb.obj");
	mesh.write_m("pcb.m");
	COctreeSMC<CTMesh> smc(&mesh);
	CTMesh *quad = smc.quad_mesh();
	quad->write_m("pcb_quad.m");
	cout << "Finished!" << endl;
}