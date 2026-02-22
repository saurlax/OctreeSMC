#ifndef _OCTREE_SMC_H_
#define _OCTREE_SMC_H_

#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include <queue>
#include <climits>
#include <iostream>
#include <chrono>

#include "ToolMesh.h"

namespace MeshLib
{
  using namespace std;

  struct OSMCInt3
  {
    int x;
    int y;
    int z;
  };

  struct OSMCEq4
  {
    int a;
    int b;
    int c;
    int d;
  };

  static const int kCornerOffset[8][3] = {
      {0, 0, 0},
      {1, 0, 0},
      {1, 1, 0},
      {0, 1, 0},
      {0, 0, 1},
      {1, 0, 1},
      {1, 1, 1},
      {0, 1, 1}};

  static const int kEdgeCorners[12][2] = {
      {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

  static const int kTriTable[256][16] = {
      {-1},
      {0, 8, 3, -1},
      {0, 1, 9, -1},
      {1, 8, 3, 9, 8, 1, -1},
      {1, 2, 10, -1},
      {0, 8, 3, 1, 2, 10, -1},
      {9, 2, 10, 0, 2, 9, -1},
      {2, 8, 3, 2, 10, 8, 10, 9, 8, -1},
      {3, 11, 2, -1},
      {0, 11, 2, 8, 11, 0, -1},
      {1, 9, 0, 2, 3, 11, -1},
      {1, 11, 2, 1, 9, 11, 9, 8, 11, -1},
      {3, 10, 1, 11, 10, 3, -1},
      {0, 10, 1, 0, 8, 10, 8, 11, 10, -1},
      {3, 9, 0, 3, 11, 9, 11, 10, 9, -1},
      {9, 8, 10, 10, 8, 11, -1},
      {4, 7, 8, -1},
      {4, 3, 0, 7, 3, 4, -1},
      {0, 1, 9, 8, 4, 7, -1},
      {4, 1, 9, 4, 7, 1, 7, 3, 1, -1},
      {1, 2, 10, 8, 4, 7, -1},
      {3, 4, 7, 3, 0, 4, 1, 2, 10, -1},
      {9, 2, 10, 9, 0, 2, 8, 4, 7, -1},
      {2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1},
      {8, 4, 7, 3, 11, 2, -1},
      {11, 4, 7, 11, 2, 4, 2, 0, 4, -1},
      {9, 0, 1, 8, 4, 7, 2, 3, 11, -1},
      {4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1},
      {3, 10, 1, 3, 11, 10, 7, 8, 4, -1},
      {1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1},
      {4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1},
      {4, 7, 11, 4, 11, 9, 9, 11, 10, -1},
      {9, 5, 4, -1},
      {9, 5, 4, 0, 8, 3, -1},
      {0, 5, 4, 1, 5, 0, -1},
      {8, 5, 4, 8, 3, 5, 3, 1, 5, -1},
      {1, 2, 10, 9, 5, 4, -1},
      {3, 0, 8, 1, 2, 10, 4, 9, 5, -1},
      {5, 2, 10, 5, 4, 2, 4, 0, 2, -1},
      {2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1},
      {9, 5, 4, 2, 3, 11, -1},
      {0, 11, 2, 0, 8, 11, 4, 9, 5, -1},
      {0, 5, 4, 0, 1, 5, 2, 3, 11, -1},
      {2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1},
      {10, 3, 11, 10, 1, 3, 9, 5, 4, -1},
      {4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1},
      {5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1},
      {5, 4, 8, 5, 8, 10, 10, 8, 11, -1},
      {9, 7, 8, 5, 7, 9, -1},
      {9, 3, 0, 9, 5, 3, 5, 7, 3, -1},
      {0, 7, 8, 0, 1, 7, 1, 5, 7, -1},
      {1, 5, 3, 3, 5, 7, -1},
      {9, 7, 8, 9, 5, 7, 10, 1, 2, -1},
      {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1},
      {8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1},
      {2, 10, 5, 2, 5, 3, 3, 5, 7, -1},
      {7, 9, 5, 7, 8, 9, 3, 11, 2, -1},
      {9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1},
      {2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1},
      {11, 2, 1, 11, 1, 7, 7, 1, 5, -1},
      {9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1},
      {5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
      {11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
      {11, 10, 5, 7, 11, 5, -1},
      {10, 6, 5, -1},
      {0, 8, 3, 5, 10, 6, -1},
      {9, 0, 1, 5, 10, 6, -1},
      {1, 8, 3, 1, 9, 8, 5, 10, 6, -1},
      {1, 6, 5, 2, 6, 1, -1},
      {1, 6, 5, 1, 2, 6, 3, 0, 8, -1},
      {9, 6, 5, 9, 0, 6, 0, 2, 6, -1},
      {5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1},
      {2, 3, 11, 10, 6, 5, -1},
      {11, 0, 8, 11, 2, 0, 10, 6, 5, -1},
      {0, 1, 9, 2, 3, 11, 5, 10, 6, -1},
      {5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1},
      {6, 3, 11, 6, 5, 3, 5, 1, 3, -1},
      {0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1},
      {3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1},
      {6, 5, 9, 6, 9, 11, 11, 9, 8, -1},
      {5, 10, 6, 4, 7, 8, -1},
      {4, 3, 0, 4, 7, 3, 6, 5, 10, -1},
      {1, 9, 0, 5, 10, 6, 8, 4, 7, -1},
      {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1},
      {6, 1, 2, 6, 5, 1, 4, 7, 8, -1},
      {1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1},
      {8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1},
      {7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
      {3, 11, 2, 7, 8, 4, 10, 6, 5, -1},
      {5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1},
      {0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1},
      {9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
      {8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1},
      {5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
      {0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
      {6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1},
      {10, 4, 9, 6, 4, 10, -1},
      {4, 10, 6, 4, 9, 10, 0, 8, 3, -1},
      {10, 0, 1, 10, 6, 0, 6, 4, 0, -1},
      {8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1},
      {1, 4, 9, 1, 2, 4, 2, 6, 4, -1},
      {3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1},
      {0, 2, 4, 4, 2, 6, -1},
      {8, 3, 2, 8, 2, 4, 4, 2, 6, -1},
      {10, 4, 9, 10, 6, 4, 11, 2, 3, -1},
      {0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1},
      {3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1},
      {6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
      {9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1},
      {8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
      {3, 11, 6, 3, 6, 0, 0, 6, 4, -1},
      {6, 4, 8, 11, 6, 8, -1},
      {7, 10, 6, 7, 8, 10, 8, 9, 10, -1},
      {0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1},
      {10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1},
      {10, 6, 7, 10, 7, 1, 1, 7, 3, -1},
      {1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1},
      {2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
      {7, 8, 0, 7, 0, 6, 6, 0, 2, -1},
      {7, 3, 2, 6, 7, 2, -1},
      {2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1},
      {2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
      {1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
      {11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1},
      {8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
      {0, 9, 1, 11, 6, 7, -1},
      {7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1},
      {7, 11, 6, -1},
      {7, 6, 11, -1},
      {3, 0, 8, 11, 7, 6, -1},
      {0, 1, 9, 11, 7, 6, -1},
      {8, 1, 9, 8, 3, 1, 11, 7, 6, -1},
      {10, 1, 2, 6, 11, 7, -1},
      {1, 2, 10, 3, 0, 8, 6, 11, 7, -1},
      {2, 9, 0, 2, 10, 9, 6, 11, 7, -1},
      {6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1},
      {7, 2, 3, 6, 2, 7, -1},
      {7, 0, 8, 7, 6, 0, 6, 2, 0, -1},
      {2, 7, 6, 2, 3, 7, 0, 1, 9, -1},
      {1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1},
      {10, 7, 6, 10, 1, 7, 1, 3, 7, -1},
      {10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1},
      {0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1},
      {7, 6, 10, 7, 10, 8, 8, 10, 9, -1},
      {6, 8, 4, 11, 8, 6, -1},
      {3, 6, 11, 3, 0, 6, 0, 4, 6, -1},
      {8, 6, 11, 8, 4, 6, 9, 0, 1, -1},
      {9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1},
      {6, 8, 4, 6, 11, 8, 2, 10, 1, -1},
      {1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1},
      {4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1},
      {10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
      {8, 2, 3, 8, 4, 2, 4, 6, 2, -1},
      {0, 4, 2, 4, 6, 2, -1},
      {1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1},
      {1, 9, 4, 1, 4, 2, 2, 4, 6, -1},
      {8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1},
      {10, 1, 0, 10, 0, 6, 6, 0, 4, -1},
      {4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
      {10, 9, 4, 6, 10, 4, -1},
      {4, 9, 5, 7, 6, 11, -1},
      {0, 8, 3, 4, 9, 5, 11, 7, 6, -1},
      {5, 0, 1, 5, 4, 0, 7, 6, 11, -1},
      {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1},
      {9, 5, 4, 10, 1, 2, 7, 6, 11, -1},
      {6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1},
      {7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1},
      {3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
      {7, 2, 3, 7, 6, 2, 5, 4, 9, -1},
      {9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1},
      {3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1},
      {6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
      {9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1},
      {1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
      {4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
      {7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1},
      {6, 9, 5, 6, 11, 9, 11, 8, 9, -1},
      {3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1},
      {0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1},
      {6, 11, 3, 6, 3, 5, 5, 3, 1, -1},
      {1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1},
      {0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
      {11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
      {6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1},
      {5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1},
      {9, 5, 6, 9, 6, 0, 0, 6, 2, -1},
      {1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
      {1, 5, 6, 2, 1, 6, -1},
      {1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
      {10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1},
      {0, 3, 8, 5, 6, 10, -1},
      {10, 5, 6, -1},
      {11, 5, 10, 7, 5, 11, -1},
      {11, 5, 10, 11, 7, 5, 8, 3, 0, -1},
      {5, 11, 7, 5, 10, 11, 1, 9, 0, -1},
      {10, 11, 5, 5, 11, 7, 9, 8, 1, 8, 3, 1, -1},
      {11, 1, 2, 11, 7, 1, 7, 5, 1, -1},
      {0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1},
      {9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1},
      {7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
      {2, 5, 10, 2, 3, 5, 3, 7, 5, -1},
      {8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1},
      {9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1},
      {9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
      {1, 3, 5, 3, 7, 5, -1},
      {0, 8, 7, 0, 7, 1, 1, 7, 5, -1},
      {9, 0, 3, 9, 3, 5, 5, 3, 7, -1},
      {9, 8, 7, 5, 9, 7, -1},
      {5, 8, 4, 5, 10, 8, 10, 11, 8, -1},
      {5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1},
      {0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1},
      {10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
      {2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1},
      {0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
      {0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
      {9, 4, 5, 2, 11, 3, -1},
      {2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1},
      {5, 10, 2, 5, 2, 4, 4, 2, 0, -1},
      {3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
      {5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1},
      {8, 4, 5, 8, 5, 3, 3, 5, 1, -1},
      {0, 4, 5, 1, 0, 5, -1},
      {8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1},
      {9, 4, 5, -1},
      {4, 11, 7, 4, 9, 11, 9, 10, 11, -1},
      {0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1},
      {1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1},
      {3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
      {4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1},
      {9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
      {11, 7, 4, 11, 4, 2, 2, 4, 0, -1},
      {11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1},
      {2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1},
      {9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
      {3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
      {1, 10, 2, 8, 7, 4, -1},
      {4, 9, 1, 4, 1, 7, 7, 1, 3, -1},
      {4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1},
      {4, 0, 3, 7, 4, 3, -1},
      {4, 8, 7, -1},
      {9, 10, 8, 10, 11, 8, -1},
      {3, 0, 9, 3, 9, 11, 11, 9, 10, -1},
      {0, 1, 10, 0, 10, 8, 8, 10, 11, -1},
      {3, 1, 10, 11, 3, 10, -1},
      {1, 2, 11, 1, 11, 9, 9, 11, 8, -1},
      {3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1},
      {0, 2, 11, 8, 0, 11, -1},
      {3, 2, 11, -1},
      {2, 3, 8, 2, 8, 10, 10, 8, 9, -1},
      {9, 10, 2, 0, 9, 2, -1},
      {2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1},
      {1, 10, 2, -1},
      {1, 3, 8, 9, 1, 8, -1},
      {0, 9, 1, -1},
      {0, 3, 8, -1},
      {-1}};

  template <typename M>
  class COctreeSMC
  {
  public:
    COctreeSMC(M *pMesh, int maxDepth = 6);
    ~COctreeSMC();
    M *quad_mesh();

  private:
    struct Tri
    {
      CPoint p0;
      CPoint p1;
      CPoint p2;
      CPoint bmin;
      CPoint bmax;
    };

    struct BoxRange
    {
      int xmin;
      int ymin;
      int zmin;
      int xmax;
      int ymax;
      int zmax;
      bool is_single() const
      {
        return xmin == xmax && ymin == ymax && zmin == zmax;
      }
    };

    struct NodeParms
    {
      unsigned char config;
      int d;
      bool valid;
      NodeParms() : config(0), d(0), valid(false) {}
    };

    struct OctreeNode
    {
      OctreeNode *children[8];
      OctreeNode *parent;
      BoxRange range;
      int indexInParent;
      int layerIndex;
      bool visited;
      NodeParms parms;

      OctreeNode()
      {
        parent = NULL;
        indexInParent = -1;
        layerIndex = 0;
        visited = false;
        for (int i = 0; i < 8; ++i)
          children[i] = NULL;
      }
      ~OctreeNode()
      {
        clear_children();
      }
      bool is_leaf() const
      {
        for (int i = 0; i < 8; ++i)
          if (children[i] != NULL)
            return false;
        return true;
      }
      void clear_children()
      {
        for (int i = 0; i < 8; ++i)
        {
          if (children[i] != NULL)
          {
            delete children[i];
            children[i] = NULL;
          }
        }
      }
    };

    struct EdgeKey
    {
      int a;
      int b;
      bool operator<(const EdgeKey &rhs) const
      {
        if (a != rhs.a)
          return a < rhs.a;
        return b < rhs.b;
      }
    };

    struct VertKey
    {
      long long x;
      long long y;
      long long z;
      bool operator<(const VertKey &rhs) const
      {
        if (x != rhs.x)
          return x < rhs.x;
        if (y != rhs.y)
          return y < rhs.y;
        return z < rhs.z;
      }
    };

  private:
    void build_root_bbox();
    void build_triangles();
    bool ray_intersect_triangle(const CPoint &orig, const CPoint &dir, const Tri &tri) const;
    bool point_in_mesh_fast(const CPoint &p) const;
    bool point_in_mesh_vote(const CPoint &p) const;
    int get_index_on(int x, int y, int z, int bitIndex) const;
    void init_child_range(OctreeNode *node, OctreeNode *parent, int index) const;
    OctreeNode *create_to_leaf(int x, int y, int z);
    void construct_tree();
    void shrink_tree();
    bool can_merge_node(OctreeNode *node, int &D) const;
    unsigned char calculate_config(OctreeNode *children[8]) const;
    int calculate_d(int cx, int cy, int cz, unsigned char config) const;
    CPoint grid_to_world(double gx, double gy, double gz) const;
    void generate_face(OctreeNode *node, M *out, int &vid, int &fid,
                       map<VertKey, typename M::CVertex *> &vmap,
                       map<EdgeKey, int> &edgeUse,
                       map<EdgeKey, int> &dirEdgeUse,
                       double quant) const;
    void generate_face_leaf(OctreeNode *node, M *out, int &vid, int &fid,
                            map<VertKey, typename M::CVertex *> &vmap,
                            map<EdgeKey, int> &edgeUse,
                            map<EdgeKey, int> &dirEdgeUse,
                            double quant) const;
    CPoint get_intersected_point_at_edge(const BoxRange &range, int edgeIndex, const OSMCInt3 &normal, int d) const;
    bool can_add_face(vector<typename M::CVertex *> &verts,
                      map<EdgeKey, int> &edgeUse,
                      map<EdgeKey, int> &dirEdgeUse) const;
    typename M::CVertex *get_vertex(const CPoint &p,
                                    M *out,
                                    int &vid,
                                    map<VertKey, typename M::CVertex *> &vmap,
                                    double quant) const;

  private:
    M *m_pMesh;
    int m_maxDepth;
    int m_scale;
    CPoint m_rootMin;
    CPoint m_rootMax;
    double m_step;
    OctreeNode *m_root;
    vector<Tri> m_tris;
    queue<OctreeNode *> m_queue;
  };

  static const int kPointDeltaCS[8][3] = {
      {0, 1, 1},
      {0, 1, 0},
      {0, 0, 0},
      {0, 0, 1},
      {1, 1, 1},
      {1, 1, 0},
      {1, 0, 0},
      {1, 0, 1}};

  static const unsigned char kPointFlagCS[8] = {
      1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7};

  static const int kVertexVoxelIndexCS[8] = {2, 6, 1, 5, 3, 7, 0, 4};
  static const int kMidVoxelIndexCS[8] = {4, 0, 7, 3, 5, 1, 6, 2};
  static const unsigned char kNormalNotSimple = 13;

  static const unsigned char kConfigToNormalTypeId[256] = {
      13, 0, 1, 2, 3, 13, 4, 13, 5, 6, 13, 13, 7, 13, 13, 8, 3, 9, 13, 13, 13, 13, 13, 13, 13, 13, 13, 0, 13, 13, 13, 13,
      5, 13, 10, 13, 13, 13, 13, 1, 13, 13, 13, 13, 13, 13, 13, 13, 7, 13, 13, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 2,
      0, 13, 13, 13, 9, 13, 13, 13, 13, 13, 13, 13, 13, 13, 3, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 6, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 4, 13, 13, 5, 13, 13, 13, 13, 10, 13, 13, 13, 13, 13, 13, 13,
      1, 1, 13, 13, 13, 13, 13, 13, 13, 10, 13, 13, 13, 13, 5, 13, 13, 4, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13,
      6, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 3, 13, 13, 13, 13, 13, 13, 13, 13, 9, 13, 13, 13, 0,
      2, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 11, 13, 13, 7, 13, 13, 13, 13, 13, 13, 13, 13, 1, 13, 13, 13, 13, 10, 13, 5,
      13, 13, 13, 13, 0, 13, 13, 13, 13, 13, 13, 13, 13, 13, 9, 3, 8, 13, 13, 7, 13, 13, 6, 5, 13, 4, 13, 3, 2, 1, 0, 13};

  static const unsigned char kConfigToEqType[256] = {
      55, 0, 1, 2, 3, 55, 4, 55, 5, 6, 55, 55, 7, 55, 55, 8, 9, 10, 55, 55, 55, 55, 55, 55, 55, 55, 55, 11, 55, 55, 55, 55,
      12, 55, 13, 55, 55, 55, 55, 14, 55, 55, 55, 55, 55, 55, 55, 55, 15, 55, 55, 16, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 17,
      18, 55, 55, 55, 19, 55, 55, 55, 55, 55, 55, 55, 55, 20, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      0, 21, 55, 55, 55, 55, 55, 22, 55, 55, 55, 55, 55, 55, 55, 55, 23, 55, 55, 24, 55, 55, 55, 55, 25, 55, 55, 55, 0, 55, 0, 0,
      26, 27, 55, 55, 55, 55, 55, 55, 55, 55, 28, 55, 55, 55, 55, 29, 55, 55, 30, 55, 55, 55, 55, 55, 55, 55, 55, 31, 55, 55, 55, 55,
      55, 32, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 0, 55, 33, 55, 55, 55, 55, 55, 0, 55, 55, 55, 34, 55, 0,
      0, 35, 36, 55, 55, 55, 55, 55, 55, 55, 55, 55, 37, 55, 55, 38, 55, 55, 55, 55, 55, 55, 55, 0, 39, 55, 55, 0, 55, 40, 0, 41,
      55, 55, 55, 55, 42, 55, 55, 0, 55, 55, 55, 0, 55, 0, 43, 44, 45, 55, 55, 46, 55, 0, 47, 48, 55, 49, 0, 50, 51, 52, 53, 55};

  static const OSMCInt3 kNormalTypeIdToNormal[13] = {
      {1, -1, -1}, {1, -1, 1}, {1, -1, 0}, {1, 1, 1}, {1, 0, 1}, {1, 1, -1}, {1, 0, -1}, {1, 1, 0}, {1, 0, 0}, {0, 1, 1}, {0, 1, -1}, {0, 1, 0}, {0, 0, 1}};

  static const OSMCEq4 kEqTypeToEqQuad[54] = {
      {1, -1, -1, -1}, {1, -1, 1, 0}, {1, -1, 0, 0}, {1, 1, 1, 1}, {1, 0, 1, 1}, {1, 1, -1, 0}, {1, 0, -1, 0}, {1, 1, 0, 1}, {1, 0, 0, 1},
      {1, 1, 1, 2}, {0, 1, 1, 1}, {1, -1, -1, 0}, {1, 1, -1, 1}, {0, 1, -1, 0}, {1, -1, 1, 1}, {1, 1, 0, 1}, {0, 1, 0, 0}, {1, -1, 0, 1},
      {1, -1, -1, 0}, {0, 1, 1, 1}, {1, 1, 1, 2}, {1, 0, -1, 0}, {0, 0, 1, 1}, {1, 0, 1, 2}, {1, 1, -1, 0}, {0, 1, -1, -1}, {1, -1, 1, 2},
      {1, -1, 1, 1}, {0, 1, -1, 0}, {1, 1, -1, 1}, {1, 0, 1, 1}, {0, 0, 1, 0}, {1, 0, -1, 1}, {1, 1, 1, 1}, {0, 1, 1, 0}, {1, -1, -1, 1},
      {1, -1, 0, 0}, {0, 1, 0, 1}, {1, 1, 0, 2}, {1, -1, 1, 0}, {0, 1, -1, 1}, {1, 1, -1, 2}, {1, -1, -1, -1}, {0, 1, 1, 2}, {1, 1, 1, 3},
      {1, 0, 0, 0}, {1, 1, 0, 0}, {1, 0, -1, -1}, {1, 1, -1, -1}, {1, 0, 1, 0}, {1, 1, 1, 0}, {1, -1, 0, -1}, {1, -1, 1, -1}, {1, -1, -1, -2}};

  template <typename M>
  COctreeSMC<M>::COctreeSMC(M *pMesh, int maxDepth)
  {
    m_pMesh = pMesh;
    m_maxDepth = maxDepth > 0 ? maxDepth : 1;
    if (m_maxDepth > 9)
      m_maxDepth = 9;
    m_scale = 1 << m_maxDepth;
    m_root = NULL;
    build_root_bbox();
    build_triangles();
  }

  template <typename M>
  COctreeSMC<M>::~COctreeSMC()
  {
    if (m_root != NULL)
      delete m_root;
  }

  template <typename M>
  void COctreeSMC<M>::build_root_bbox()
  {
    CPoint pmin(1e30, 1e30, 1e30);
    CPoint pmax(-1e30, -1e30, -1e30);
    for (typename M::MeshVertexIterator mv(m_pMesh); !mv.end(); mv++)
    {
      CPoint p = mv.value()->point();
      for (int i = 0; i < 3; ++i)
      {
        if (p[i] < pmin[i])
          pmin[i] = p[i];
        if (p[i] > pmax[i])
          pmax[i] = p[i];
      }
    }
    CPoint size = pmax - pmin;
    double max_len = size[0];
    if (size[1] > max_len)
      max_len = size[1];
    if (size[2] > max_len)
      max_len = size[2];
    if (max_len <= 1e-12)
      max_len = 1.0;
    CPoint center = (pmin + pmax) / 2.0;
    CPoint half(max_len * 0.5, max_len * 0.5, max_len * 0.5);
    m_rootMin = center - half;
    m_rootMax = center + half;
    m_step = max_len / static_cast<double>(m_scale);
  }

  template <typename M>
  void COctreeSMC<M>::build_triangles()
  {
    m_tris.clear();
    vector<typename M::CVertex *> verts;
    for (typename M::MeshFaceIterator mf(m_pMesh); !mf.end(); mf++)
    {
      verts.clear();
      for (typename M::FaceVertexIterator fv(mf.value()); !fv.end(); fv++)
        verts.push_back(fv.value());
      if (verts.size() < 3)
        continue;
      CPoint p0 = verts[0]->point();
      for (size_t i = 1; i + 1 < verts.size(); ++i)
      {
        Tri tri;
        tri.p0 = p0;
        tri.p1 = verts[i]->point();
        tri.p2 = verts[i + 1]->point();
        tri.bmin = tri.p0;
        tri.bmax = tri.p0;
        for (int k = 0; k < 3; ++k)
        {
          if (tri.p1[k] < tri.bmin[k])
            tri.bmin[k] = tri.p1[k];
          if (tri.p2[k] < tri.bmin[k])
            tri.bmin[k] = tri.p2[k];
          if (tri.p1[k] > tri.bmax[k])
            tri.bmax[k] = tri.p1[k];
          if (tri.p2[k] > tri.bmax[k])
            tri.bmax[k] = tri.p2[k];
        }
        m_tris.push_back(tri);
      }
    }
  }

  template <typename M>
  bool COctreeSMC<M>::ray_intersect_triangle(const CPoint &orig, const CPoint &dir, const Tri &tri) const
  {
    const double eps = 1e-9;
    CPoint v0v1 = tri.p1 - tri.p0;
    CPoint v0v2 = tri.p2 - tri.p0;
    CPoint pvec = dir ^ v0v2;
    double det = v0v1 * pvec;
    if (fabs(det) < eps)
      return false;
    double invDet = 1.0 / det;
    CPoint tvec = orig - tri.p0;
    double u = (tvec * pvec) * invDet;
    if (u < 0.0 || u > 1.0)
      return false;
    CPoint qvec = tvec ^ v0v1;
    double v = (dir * qvec) * invDet;
    if (v < 0.0 || (u + v) > 1.0)
      return false;
    double t = (v0v2 * qvec) * invDet;
    return t > eps;
  }

  template <typename M>
  bool COctreeSMC<M>::point_in_mesh_fast(const CPoint &p) const
  {
    const CPoint dir(1, 0, 0);
    int hits = 0;
    for (size_t t = 0; t < m_tris.size(); ++t)
    {
      const Tri &tri = m_tris[t];
      if (p[1] < tri.bmin[1] || p[1] > tri.bmax[1])
        continue;
      if (p[2] < tri.bmin[2] || p[2] > tri.bmax[2])
        continue;
      if (p[0] > tri.bmax[0])
        continue;
      if (ray_intersect_triangle(p, dir, tri))
        hits++;
    }
    return (hits % 2) == 1;
  }

  template <typename M>
  bool COctreeSMC<M>::point_in_mesh_vote(const CPoint &p) const
  {
    const CPoint dirs[3] = {CPoint(1, 0, 0), CPoint(0, 1, 0), CPoint(0, 0, 1)};
    int insideCount = 0;
    for (int i = 0; i < 3; ++i)
    {
      int hits = 0;
      for (size_t t = 0; t < m_tris.size(); ++t)
      {
        const Tri &tri = m_tris[t];
        if (i == 0)
        {
          if (p[1] < tri.bmin[1] || p[1] > tri.bmax[1] || p[2] < tri.bmin[2] || p[2] > tri.bmax[2] || p[0] > tri.bmax[0])
            continue;
        }
        else if (i == 1)
        {
          if (p[0] < tri.bmin[0] || p[0] > tri.bmax[0] || p[2] < tri.bmin[2] || p[2] > tri.bmax[2] || p[1] > tri.bmax[1])
            continue;
        }
        else
        {
          if (p[0] < tri.bmin[0] || p[0] > tri.bmax[0] || p[1] < tri.bmin[1] || p[1] > tri.bmax[1] || p[2] > tri.bmax[2])
            continue;
        }
        if (ray_intersect_triangle(p, dirs[i], m_tris[t]))
          hits++;
      }
      if ((hits % 2) == 1)
        insideCount++;
    }
    return insideCount >= 2;
  }

  template <typename M>
  int COctreeSMC<M>::get_index_on(int x, int y, int z, int bitIndex) const
  {
    int ret = 0;
    if ((x & (1 << bitIndex)) != 0)
      ret |= 1;
    if ((y & (1 << bitIndex)) != 0)
      ret |= 2;
    if ((z & (1 << bitIndex)) != 0)
      ret |= 4;
    return ret;
  }

  template <typename M>
  void COctreeSMC<M>::init_child_range(OctreeNode *node, OctreeNode *parent, int index) const
  {
    int dx = (parent->range.xmax - parent->range.xmin + 1) >> 1;
    int dy = (parent->range.ymax - parent->range.ymin + 1) >> 1;
    int dz = (parent->range.zmax - parent->range.zmin + 1) >> 1;
    if ((index & 1) == 0)
    {
      node->range.xmin = parent->range.xmin;
      node->range.xmax = parent->range.xmin + dx - 1;
    }
    else
    {
      node->range.xmin = parent->range.xmin + dx;
      node->range.xmax = parent->range.xmax;
    }
    if ((index & 2) == 0)
    {
      node->range.ymin = parent->range.ymin;
      node->range.ymax = parent->range.ymin + dy - 1;
    }
    else
    {
      node->range.ymin = parent->range.ymin + dy;
      node->range.ymax = parent->range.ymax;
    }
    if ((index & 4) == 0)
    {
      node->range.zmin = parent->range.zmin;
      node->range.zmax = parent->range.zmin + dz - 1;
    }
    else
    {
      node->range.zmin = parent->range.zmin + dz;
      node->range.zmax = parent->range.zmax;
    }
  }

  template <typename M>
  typename COctreeSMC<M>::OctreeNode *COctreeSMC<M>::create_to_leaf(int x, int y, int z)
  {
    OctreeNode *node = m_root;
    for (int i = 1; i <= m_maxDepth; ++i)
    {
      int idx = get_index_on(x, y, z, m_maxDepth - i);
      if (node->children[idx] == NULL)
      {
        OctreeNode *child = new OctreeNode();
        child->parent = node;
        child->indexInParent = idx;
        child->layerIndex = node->layerIndex - 1;
        init_child_range(child, node, idx);
        node->children[idx] = child;
      }
      node = node->children[idx];
    }
    return node;
  }

  template <typename M>
  int COctreeSMC<M>::calculate_d(int cx, int cy, int cz, unsigned char config) const
  {
    unsigned char eq = kConfigToEqType[config];
    if (eq >= 54)
      return INT_MIN;
    const OSMCEq4 &e = kEqTypeToEqQuad[eq];
    return e.d + e.a * cx + e.b * cy + e.c * cz;
  }

  template <typename M>
  unsigned char COctreeSMC<M>::calculate_config(OctreeNode *children[8]) const
  {
    unsigned char firstc = 0;
    int firstIndex = -1;
    for (int i = 0; i < 8; ++i)
    {
      if (children[i] != NULL && children[i]->parms.valid)
      {
        firstc = children[i]->parms.config;
        firstIndex = i;
        break;
      }
    }
    if (firstIndex < 0)
      return 0;

    unsigned char midValue = ((firstc & kPointFlagCS[kMidVoxelIndexCS[firstIndex]]) == 0) ? 0 : 255;
    int ret = 0;
    for (int i = 0; i < 8; ++i)
    {
      unsigned char cfg = midValue;
      if (children[i] != NULL && children[i]->parms.valid)
        cfg = children[i]->parms.config;
      unsigned char flag = kPointFlagCS[kVertexVoxelIndexCS[i]];
      ret |= (cfg & flag);
    }
    return static_cast<unsigned char>(ret);
  }

  template <typename M>
  bool COctreeSMC<M>::can_merge_node(OctreeNode *node, int &D) const
  {
    unsigned char normalType = kNormalNotSimple;
    bool found = false;
    for (int i = 0; i < 8; ++i)
    {
      OctreeNode *c = node->children[i];
      if (c != NULL)
      {
        if (!c->parms.valid)
          return false;
        unsigned char nt = kConfigToNormalTypeId[c->parms.config];
        if (nt == kNormalNotSimple)
          return false;
        if (!found)
        {
          found = true;
          normalType = nt;
          D = c->parms.d;
        }
      }
    }
    if (!found)
      return false;
    for (int i = 0; i < 8; ++i)
    {
      OctreeNode *c = node->children[i];
      if (c != NULL)
      {
        unsigned char nt = kConfigToNormalTypeId[c->parms.config];
        if (nt != normalType || c->parms.d != D)
          return false;
      }
    }
    return true;
  }

  template <typename M>
  CPoint COctreeSMC<M>::grid_to_world(double gx, double gy, double gz) const
  {
    return CPoint(m_rootMin[0] + gx * m_step,
                  m_rootMin[1] + gy * m_step,
                  m_rootMin[2] + gz * m_step);
  }

  template <typename M>
  void COctreeSMC<M>::construct_tree()
  {
    long long totalCells = static_cast<long long>(m_scale) * m_scale * m_scale;
    long long processed = 0;
    long long boundaryCells = 0;
    int zLogStep = m_scale / 20;
    if (zLogStep < 1)
      zLogStep = 1;
    long long cellLogStep = totalCells / 200;
    if (cellLogStep < 1000)
      cellLogStep = 1000;

    int gp = m_scale + 1;
    vector<signed char> pointState(static_cast<size_t>(gp) * gp * gp, -1);
    auto pointIndex = [&](int gx, int gy, int gz) -> size_t
    {
      return (static_cast<size_t>(gz) * gp + gy) * gp + gx;
    };
    auto pointInsideCached = [&](int gx, int gy, int gz) -> bool
    {
      size_t idx = pointIndex(gx, gy, gz);
      signed char &st = pointState[idx];
      if (st < 0)
      {
        CPoint p = grid_to_world(static_cast<double>(gx), static_cast<double>(gy), static_cast<double>(gz));
        st = point_in_mesh_fast(p) ? 1 : 0;
      }
      return st > 0;
    };

    cout << "[OctreeSMC] ConstructTree start, cells=" << totalCells << endl;

    for (int z = 0; z < m_scale; ++z)
    {
      for (int y = 0; y < m_scale; ++y)
      {
        for (int x = 0; x < m_scale; ++x)
        {
          unsigned char value = 0;
          for (int pi = 0; pi < 8; ++pi)
          {
            int gx = x + kPointDeltaCS[pi][0];
            int gy = y + kPointDeltaCS[pi][1];
            int gz = z + kPointDeltaCS[pi][2];
            if (pointInsideCached(gx, gy, gz))
              value |= kPointFlagCS[pi];
          }
          if (value != 0 && value != 255)
          {
            OctreeNode *leaf = create_to_leaf(x, y, z);
            leaf->parms.valid = true;
            leaf->parms.config = value;
            leaf->parms.d = calculate_d(x, y, z, value);
            boundaryCells++;
            leaf->visited = true;
            if (leaf->parent != NULL && !leaf->parent->visited)
            {
              leaf->parent->visited = true;
              m_queue.push(leaf->parent);
            }
          }
          processed++;
          if ((processed % cellLogStep) == 0)
          {
            double pct2 = totalCells > 0 ? (100.0 * processed / totalCells) : 100.0;
            cout << "[OctreeSMC] ConstructTree fine progress " << pct2 << "% ("
                 << processed << "/" << totalCells << ")" << endl;
          }
        }
      }
      if (((z + 1) % zLogStep) == 0 || z + 1 == m_scale)
      {
        double pct = totalCells > 0 ? (100.0 * processed / totalCells) : 100.0;
        cout << "[OctreeSMC] ConstructTree progress " << pct << "% (z=" << (z + 1) << "/" << m_scale
             << ", boundary=" << boundaryCells << ")" << endl;
      }
    }
    cout << "[OctreeSMC] ConstructTree done, boundary cells=" << boundaryCells << endl;
  }

  template <typename M>
  void COctreeSMC<M>::shrink_tree()
  {
    long long popped = 0;
    long long merged = 0;
    cout << "[OctreeSMC] Shrink start, initial queue=" << m_queue.size() << endl;
    while (!m_queue.empty())
    {
      OctreeNode *node = m_queue.front();
      m_queue.pop();
      popped++;
      node->visited = false;
      int D = INT_MIN;
      if (can_merge_node(node, D))
      {
        node->parms.valid = true;
        node->parms.config = calculate_config(node->children);
        node->parms.d = D;
        node->clear_children();
        merged++;
        if (node->parent != NULL && !node->parent->visited)
        {
          node->parent->visited = true;
          m_queue.push(node->parent);
        }
      }
      if ((popped % 5000) == 0)
      {
        cout << "[OctreeSMC] Shrink progress popped=" << popped << ", merged=" << merged
             << ", queue=" << m_queue.size() << endl;
      }
    }
    cout << "[OctreeSMC] Shrink done, popped=" << popped << ", merged=" << merged << endl;
  }

  template <typename M>
  CPoint COctreeSMC<M>::get_intersected_point_at_edge(const BoxRange &range, int edgeIndex, const OSMCInt3 &normal, int d) const
  {
    double x = 0, y = 0, z = 0;
    switch (edgeIndex)
    {
    case 0:
      x = range.xmin;
      y = range.ymax + 1;
      z = normal.z != 0 ? static_cast<double>(d - normal.x * x - normal.y * y) / normal.z : range.zmin;
      break;
    case 2:
      x = range.xmin;
      y = range.ymin;
      z = normal.z != 0 ? static_cast<double>(d - normal.x * x - normal.y * y) / normal.z : range.zmin;
      break;
    case 4:
      x = range.xmax + 1;
      y = range.ymax + 1;
      z = normal.z != 0 ? static_cast<double>(d - normal.x * x - normal.y * y) / normal.z : range.zmin;
      break;
    case 6:
      x = range.xmax + 1;
      y = range.ymin;
      z = normal.z != 0 ? static_cast<double>(d - normal.x * x - normal.y * y) / normal.z : range.zmin;
      break;
    case 8:
      y = range.ymax + 1;
      z = range.zmax + 1;
      x = normal.x != 0 ? static_cast<double>(d - normal.y * y - normal.z * z) / normal.x : range.xmin;
      break;
    case 9:
      y = range.ymax + 1;
      z = range.zmin;
      x = normal.x != 0 ? static_cast<double>(d - normal.y * y - normal.z * z) / normal.x : range.xmin;
      break;
    case 10:
      y = range.ymin;
      z = range.zmin;
      x = normal.x != 0 ? static_cast<double>(d - normal.y * y - normal.z * z) / normal.x : range.xmin;
      break;
    case 11:
      y = range.ymin;
      z = range.zmax + 1;
      x = normal.x != 0 ? static_cast<double>(d - normal.y * y - normal.z * z) / normal.x : range.xmin;
      break;
    case 1:
      x = range.xmin;
      z = range.zmin;
      y = normal.y != 0 ? static_cast<double>(d - normal.x * x - normal.z * z) / normal.y : range.ymin;
      break;
    case 3:
      x = range.xmin;
      z = range.zmax + 1;
      y = normal.y != 0 ? static_cast<double>(d - normal.x * x - normal.z * z) / normal.y : range.ymin;
      break;
    case 5:
      x = range.xmax + 1;
      z = range.zmin;
      y = normal.y != 0 ? static_cast<double>(d - normal.x * x - normal.z * z) / normal.y : range.ymin;
      break;
    default:
      x = range.xmax + 1;
      z = range.zmax + 1;
      y = normal.y != 0 ? static_cast<double>(d - normal.x * x - normal.z * z) / normal.y : range.ymin;
      break;
    }
    return grid_to_world(x, y, z);
  }

  template <typename M>
  typename M::CVertex *COctreeSMC<M>::get_vertex(const CPoint &p,
                                                 M *out,
                                                 int &vid,
                                                 map<VertKey, typename M::CVertex *> &vmap,
                                                 double quant) const
  {
    VertKey k;
    k.x = static_cast<long long>(llround(p[0] * quant));
    k.y = static_cast<long long>(llround(p[1] * quant));
    k.z = static_cast<long long>(llround(p[2] * quant));
    typename M::CVertex *&v = vmap[k];
    if (v == NULL)
    {
      v = out->createVertex(vid++);
      v->point() = p;
    }
    return v;
  }

  template <typename M>
  bool COctreeSMC<M>::can_add_face(vector<typename M::CVertex *> &verts,
                                   map<EdgeKey, int> &edgeUse,
                                   map<EdgeKey, int> &dirEdgeUse) const
  {
    size_t n = verts.size();
    if (n < 3)
      return false;
    for (size_t i = 0; i < n; ++i)
    {
      int a = verts[i]->id();
      int b = verts[(i + 1) % n]->id();
      if (a == b)
        return false;
      EdgeKey und{a < b ? a : b, a < b ? b : a};
      if (edgeUse[und] >= 2)
        return false;
    }
    bool needFlip = false;
    for (size_t i = 0; i < n; ++i)
    {
      int a = verts[i]->id();
      int b = verts[(i + 1) % n]->id();
      EdgeKey dir{a, b};
      if (dirEdgeUse[dir] > 0)
      {
        needFlip = true;
        break;
      }
    }
    if (needFlip)
      reverse(verts.begin() + 1, verts.end());

    for (size_t i = 0; i < n; ++i)
    {
      int a = verts[i]->id();
      int b = verts[(i + 1) % n]->id();
      EdgeKey dir{a, b};
      if (dirEdgeUse[dir] > 0)
        return false;
    }

    for (size_t i = 0; i < n; ++i)
    {
      int a = verts[i]->id();
      int b = verts[(i + 1) % n]->id();
      EdgeKey und{a < b ? a : b, a < b ? b : a};
      edgeUse[und] += 1;
      EdgeKey dir{a, b};
      dirEdgeUse[dir] += 1;
    }
    return true;
  }

  template <typename M>
  void COctreeSMC<M>::generate_face(OctreeNode *node, M *out, int &vid, int &fid,
                                    map<VertKey, typename M::CVertex *> &vmap,
                                    map<EdgeKey, int> &edgeUse,
                                    map<EdgeKey, int> &dirEdgeUse,
                                    double quant) const
  {
    unsigned char cfg = node->parms.config;
    int nt = kConfigToNormalTypeId[cfg];
    if (nt >= static_cast<int>(kNormalNotSimple))
    {
      generate_face_leaf(node, out, vid, fid, vmap, edgeUse, dirEdgeUse, quant);
      return;
    }
    const OSMCInt3 &normal = kNormalTypeIdToNormal[nt];

    CPoint corners[8];
    for (int k = 0; k < 8; ++k)
    {
      double gx = node->range.xmin + kCornerOffset[k][0] * (node->range.xmax - node->range.xmin + 1);
      double gy = node->range.ymin + kCornerOffset[k][1] * (node->range.ymax - node->range.ymin + 1);
      double gz = node->range.zmin + kCornerOffset[k][2] * (node->range.zmax - node->range.zmin + 1);
      corners[k] = grid_to_world(gx, gy, gz);
    }
    CPoint edgeMid[12];
    for (int e = 0; e < 12; ++e)
    {
      int a = kEdgeCorners[e][0];
      int b = kEdgeCorners[e][1];
      edgeMid[e] = (corners[a] + corners[b]) * 0.5;
    }
    CPoint pmin = corners[0];
    CPoint pmax = corners[6];
    const double tol = 1e-9;
    auto valid_point = [&](const CPoint &p) -> bool
    {
      if (!(p[0] == p[0] && p[1] == p[1] && p[2] == p[2]))
        return false;
      if (p[0] < pmin[0] - tol || p[0] > pmax[0] + tol)
        return false;
      if (p[1] < pmin[1] - tol || p[1] > pmax[1] + tol)
        return false;
      if (p[2] < pmin[2] - tol || p[2] > pmax[2] + tol)
        return false;
      return true;
    };

    for (int i = 0; kTriTable[cfg][i] != -1; i += 3)
    {
      int e0 = kTriTable[cfg][i];
      int e1 = kTriTable[cfg][i + 1];
      int e2 = kTriTable[cfg][i + 2];
      CPoint p0 = get_intersected_point_at_edge(node->range, e0, normal, node->parms.d);
      CPoint p1 = get_intersected_point_at_edge(node->range, e1, normal, node->parms.d);
      CPoint p2 = get_intersected_point_at_edge(node->range, e2, normal, node->parms.d);

      if (!valid_point(p0))
        p0 = edgeMid[e0];
      if (!valid_point(p1))
        p1 = edgeMid[e1];
      if (!valid_point(p2))
        p2 = edgeMid[e2];

      CPoint n = (p1 - p0) ^ (p2 - p0);
      if (n.norm() <= 1e-10)
        continue;
      vector<typename M::CVertex *> tri;
      tri.push_back(get_vertex(p0, out, vid, vmap, quant));
      tri.push_back(get_vertex(p1, out, vid, vmap, quant));
      tri.push_back(get_vertex(p2, out, vid, vmap, quant));
      if (can_add_face(tri, edgeUse, dirEdgeUse))
        out->createFace(tri, fid++);
    }
  }

  template <typename M>
  void COctreeSMC<M>::generate_face_leaf(OctreeNode *node, M *out, int &vid, int &fid,
                                         map<VertKey, typename M::CVertex *> &vmap,
                                         map<EdgeKey, int> &edgeUse,
                                         map<EdgeKey, int> &dirEdgeUse,
                                         double quant) const
  {
    unsigned char cfg = node->parms.config;
    if (cfg == 0 || cfg == 255)
      return;

    CPoint corners[8];
    for (int k = 0; k < 8; ++k)
    {
      double gx = node->range.xmin + kCornerOffset[k][0];
      double gy = node->range.ymin + kCornerOffset[k][1];
      double gz = node->range.zmin + kCornerOffset[k][2];
      corners[k] = grid_to_world(gx, gy, gz);
    }
    CPoint edgePts[12];
    for (int e = 0; e < 12; ++e)
    {
      int a = kEdgeCorners[e][0];
      int b = kEdgeCorners[e][1];
      edgePts[e] = (corners[a] + corners[b]) * 0.5;
    }
    CPoint cellCenter = grid_to_world(node->range.xmin + 0.5, node->range.ymin + 0.5, node->range.zmin + 0.5);

    for (int i = 0; kTriTable[cfg][i] != -1; i += 3)
    {
      CPoint p0 = edgePts[kTriTable[cfg][i]];
      CPoint p1 = edgePts[kTriTable[cfg][i + 1]];
      CPoint p2 = edgePts[kTriTable[cfg][i + 2]];
      CPoint n = (p1 - p0) ^ (p2 - p0);
      if (n.norm() <= 1e-10)
        continue;
      CPoint triCenter = (p0 + p1 + p2) / 3.0;
      if ((n * (triCenter - cellCenter)) < 0)
      {
        CPoint t = p1;
        p1 = p2;
        p2 = t;
      }
      vector<typename M::CVertex *> tri;
      tri.push_back(get_vertex(p0, out, vid, vmap, quant));
      tri.push_back(get_vertex(p1, out, vid, vmap, quant));
      tri.push_back(get_vertex(p2, out, vid, vmap, quant));
      if (can_add_face(tri, edgeUse, dirEdgeUse))
        out->createFace(tri, fid++);
    }
  }

  template <typename M>
  M *COctreeSMC<M>::quad_mesh()
  {
    using Clock = std::chrono::steady_clock;
    auto tStart = Clock::now();

    M *out = new M();
    int vid = 1;
    int fid = 1;

    if (m_root != NULL)
      delete m_root;
    m_root = new OctreeNode();
    m_root->range.xmin = 0;
    m_root->range.ymin = 0;
    m_root->range.zmin = 0;
    m_root->range.xmax = m_scale - 1;
    m_root->range.ymax = m_scale - 1;
    m_root->range.zmax = m_scale - 1;
    m_root->layerIndex = m_maxDepth;

    while (!m_queue.empty())
      m_queue.pop();

    auto t0 = Clock::now();
    construct_tree();
    auto t1 = Clock::now();
    shrink_tree();
    auto t2 = Clock::now();

    map<VertKey, typename M::CVertex *> vmap;
    map<EdgeKey, int> edgeUse;
    map<EdgeKey, int> dirEdgeUse;
    double quant = 1e8;

    queue<OctreeNode *> bfs;
    bfs.push(m_root);
    long long visitedNodes = 0;
    long long visitedLeaves = 0;
    cout << "[OctreeSMC] Extract start" << endl;
    while (!bfs.empty())
    {
      OctreeNode *node = bfs.front();
      bfs.pop();
      visitedNodes++;
      if (node->is_leaf())
      {
        visitedLeaves++;
        if (!node->parms.valid)
          continue;
        if (node->range.is_single())
          generate_face_leaf(node, out, vid, fid, vmap, edgeUse, dirEdgeUse, quant);
        else
          generate_face(node, out, vid, fid, vmap, edgeUse, dirEdgeUse, quant);
      }
      else
      {
        for (int i = 0; i < 8; ++i)
          if (node->children[i] != NULL)
            bfs.push(node->children[i]);
      }
      if ((visitedNodes % 20000) == 0)
      {
        cout << "[OctreeSMC] Extract progress nodes=" << visitedNodes << ", leaves=" << visitedLeaves
             << ", faces=" << (fid - 1) << endl;
      }
    }
    auto t3 = Clock::now();

    auto msConstruct = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    auto msShrink = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    auto msExtract = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
    auto msTotal = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - tStart).count();
    cout << "[OctreeSMC] Extract done, nodes=" << visitedNodes << ", leaves=" << visitedLeaves
         << ", faces=" << (fid - 1) << ", verts=" << (vid - 1) << endl;
    cout << "[OctreeSMC] Timing(ms): construct=" << msConstruct
         << ", shrink=" << msShrink
         << ", extract=" << msExtract
         << ", total=" << msTotal << endl;

    return out;
  }
}

#endif
