#include <iostream>

#include "ToolMesh.h"

using namespace std;
using namespace MeshLib;

void main(int argc, char **argv)
{
	CTMesh mesh;
	mesh.read_m("eight.m");
	cout << "Finished!" << endl;
}