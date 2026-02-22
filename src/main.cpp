#include <iostream>

#include "ToolMesh.h"
#include "OctreeSMC.h"

using namespace std;
using namespace MeshLib;

void main(int argc, char **argv)
{
	double radius = 1.0;
	auto sphere = [radius](const CPoint &p)
	{
		return p[0] * p[0] + p[1] * p[1] + p[2] * p[2] - radius * radius;
	};

	CPoint bboxMin(-1.5, -1.5, -1.5);
	CPoint bboxMax(1.5, 1.5, 1.5);

	COctreeSMC smc(sphere, 0.0, bboxMin, bboxMax, 6);
	CTMesh *mesh = smc.gen_mesh();
	mesh->write_m("sphere.m");
	mesh->write_obj("sphere.obj");
	cout << "Finished!" << endl;
}