#ifndef _OCTREE_SMC_H_
#define _OCTREE_SMC_H_

#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>

#include "ToolMesh.h"

namespace MeshLib
{
  using namespace std;

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
    ~COctreeSMC() {};

    M *quad_mesh();

  private:
    struct OctreeNode
    {
      CPoint min;
      CPoint max;
      int depth;
      OctreeNode *children[8];
      vector<typename M::CFace *> faces;
      vector<int> tris;
      bool boundary;
      bool inside;

      OctreeNode(const CPoint &pmin, const CPoint &pmax, int d)
          : min(pmin), max(pmax), depth(d), boundary(false), inside(false)
      {
        for (int i = 0; i < 8; ++i)
        {
          children[i] = NULL;
        }
      }

      bool is_leaf() const
      {
        for (int i = 0; i < 8; ++i)
        {
          if (children[i] != NULL)
            return false;
        }
        return true;
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

    struct FacePair
    {
      typename M::CFace *f1;
      typename M::CFace *f2;
    };

    struct Tri
    {
      CPoint p0;
      CPoint p1;
      CPoint p2;
      CPoint bmin;
      CPoint bmax;
    };

    struct CellKey
    {
      int x;
      int y;
      int z;
      bool operator<(const CellKey &rhs) const
      {
        if (x != rhs.x)
          return x < rhs.x;
        if (y != rhs.y)
          return y < rhs.y;
        return z < rhs.z;
      }
    };

    struct VertKey
    {
      int x;
      int y;
      int z;
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
    OctreeNode *build_root();
    void build_triangles(vector<Tri> &tris) const;
    void build_octree(OctreeNode *node, const vector<int> &tri_ids, int maxDepth, int maxTris);
    bool tri_box_overlap(const Tri &tri, const CPoint &bmin, const CPoint &bmax) const;
    void classify_leaves(OctreeNode *node, const vector<Tri> &tris);
    bool point_in_mesh(const CPoint &p, const vector<Tri> &tris) const;
    bool ray_intersect_triangle(const CPoint &orig, const CPoint &dir, const Tri &tri) const;
    bool point_in_mesh_vote(const CPoint &p, const vector<Tri> &tris) const;
    OctreeNode *find_leaf(OctreeNode *node, const CPoint &p) const;
    void insert_face(OctreeNode *node, typename M::CFace *face, const CPoint &centroid);
    void collect_leaves(OctreeNode *node, vector<OctreeNode *> &leaves);
    CPoint face_centroid(typename M::CFace *face) const;
    void face_vertices(typename M::CFace *face, vector<typename M::CVertex *> &verts) const;
    bool are_coplanar(const vector<typename M::CVertex *> &a, const vector<typename M::CVertex *> &b) const;
    bool build_quad(const vector<typename M::CVertex *> &a, const vector<typename M::CVertex *> &b,
                    vector<typename M::CVertex *> &quad) const;
    bool valid_triangle(const vector<typename M::CVertex *> &verts) const;
    bool valid_quad(const vector<typename M::CVertex *> &verts, const CPoint &normal) const;
    typename M::CVertex *get_or_create_vertex(M *out, typename M::CVertex *v, map<typename M::CVertex *, typename M::CVertex *> &vmap, int &vid) const;

  private:
    M *m_pMesh;
    int m_maxDepth;
    OctreeNode *m_root;
    vector<Tri> m_tris;
  };

  template <typename M>
  COctreeSMC<M>::COctreeSMC(M *pMesh, int maxDepth)
  {
    m_pMesh = pMesh;
    m_maxDepth = maxDepth > 0 ? maxDepth : 1;
    m_root = build_root();
    for (typename M::MeshFaceIterator mf(m_pMesh); !mf.end(); mf++)
    {
      insert_face(m_root, mf.value(), face_centroid(mf.value()));
    }
  }

  template <typename M>
  typename COctreeSMC<M>::OctreeNode *COctreeSMC<M>::build_root()
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

    if (max_len <= 0)
      max_len = 1.0;

    CPoint center = (pmin + pmax) / 2.0;
    CPoint half(max_len * 0.5, max_len * 0.5, max_len * 0.5);
    return new OctreeNode(center - half, center + half, 0);
  }

  template <typename M>
  void COctreeSMC<M>::build_triangles(vector<Tri> &tris) const
  {
    vector<typename M::CVertex *> verts;
    for (typename M::MeshFaceIterator mf(m_pMesh); !mf.end(); mf++)
    {
      face_vertices(mf.value(), verts);
      if (verts.size() < 3)
        continue;
      CPoint p0 = verts[0]->point();
      for (size_t i = 1; i + 1 < verts.size(); ++i)
      {
        CPoint p1 = verts[i]->point();
        CPoint p2 = verts[i + 1]->point();
        Tri tri;
        tri.p0 = p0;
        tri.p1 = p1;
        tri.p2 = p2;
        tri.bmin = p0;
        tri.bmax = p0;
        for (int k = 0; k < 3; ++k)
        {
          if (p1[k] < tri.bmin[k])
            tri.bmin[k] = p1[k];
          if (p2[k] < tri.bmin[k])
            tri.bmin[k] = p2[k];
          if (p1[k] > tri.bmax[k])
            tri.bmax[k] = p1[k];
          if (p2[k] > tri.bmax[k])
            tri.bmax[k] = p2[k];
        }
        tris.push_back(tri);
      }
    }
  }

  template <typename M>
  bool COctreeSMC<M>::tri_box_overlap(const Tri &tri, const CPoint &bmin, const CPoint &bmax) const
  {
    for (int k = 0; k < 3; ++k)
    {
      if (tri.bmax[k] < bmin[k] || tri.bmin[k] > bmax[k])
        return false;
    }
    return true;
  }

  template <typename M>
  void COctreeSMC<M>::build_octree(OctreeNode *node, const vector<int> &tri_ids, int maxDepth, int maxTris)
  {
    node->tris = tri_ids;
    if (node->depth >= maxDepth || static_cast<int>(tri_ids.size()) <= maxTris)
      return;

    CPoint mid = (node->min + node->max) / 2.0;
    vector<int> child_tris[8];
    for (size_t i = 0; i < tri_ids.size(); ++i)
    {
      const Tri &tri = m_tris[tri_ids[i]];
      for (int c = 0; c < 8; ++c)
      {
        CPoint cmin = node->min;
        CPoint cmax = node->max;
        if (c & 1)
          cmin[0] = mid[0];
        else
          cmax[0] = mid[0];
        if (c & 2)
          cmin[1] = mid[1];
        else
          cmax[1] = mid[1];
        if (c & 4)
          cmin[2] = mid[2];
        else
          cmax[2] = mid[2];
        if (tri_box_overlap(tri, cmin, cmax))
          child_tris[c].push_back(tri_ids[i]);
      }
    }

    for (int c = 0; c < 8; ++c)
    {
      CPoint cmin = node->min;
      CPoint cmax = node->max;
      if (c & 1)
        cmin[0] = mid[0];
      else
        cmax[0] = mid[0];
      if (c & 2)
        cmin[1] = mid[1];
      else
        cmax[1] = mid[1];
      if (c & 4)
        cmin[2] = mid[2];
      else
        cmax[2] = mid[2];
      node->children[c] = new OctreeNode(cmin, cmax, node->depth + 1);
      build_octree(node->children[c], child_tris[c], maxDepth, maxTris);
    }
    node->tris.clear();
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
    if (u < 0 || u > 1)
      return false;
    CPoint qvec = tvec ^ v0v1;
    double v = (dir * qvec) * invDet;
    if (v < 0 || u + v > 1)
      return false;
    double t = (v0v2 * qvec) * invDet;
    return t > eps;
  }

  template <typename M>
  bool COctreeSMC<M>::point_in_mesh(const CPoint &p, const vector<Tri> &tris) const
  {
    CPoint dir(1, 0, 0);
    int hits = 0;
    for (size_t i = 0; i < tris.size(); ++i)
    {
      if (ray_intersect_triangle(p, dir, tris[i]))
        hits++;
    }
    return (hits % 2) == 1;
  }

  template <typename M>
  bool COctreeSMC<M>::point_in_mesh_vote(const CPoint &p, const vector<Tri> &tris) const
  {
    const CPoint dirs[3] = {CPoint(1, 0, 0), CPoint(0, 1, 0), CPoint(0, 0, 1)};
    int inside_count = 0;
    for (int i = 0; i < 3; ++i)
    {
      int hits = 0;
      for (size_t t = 0; t < tris.size(); ++t)
      {
        if (ray_intersect_triangle(p, dirs[i], tris[t]))
          hits++;
      }
      if ((hits % 2) == 1)
        inside_count++;
    }
    return inside_count >= 2;
  }

  template <typename M>
  void COctreeSMC<M>::classify_leaves(OctreeNode *node, const vector<Tri> &tris)
  {
    if (node == NULL)
      return;
    if (node->is_leaf())
    {
      node->boundary = !node->tris.empty();
      if (!node->boundary)
      {
        CPoint center = (node->min + node->max) / 2.0;
        node->inside = point_in_mesh(center, tris);
      }
      return;
    }
    for (int i = 0; i < 8; ++i)
      classify_leaves(node->children[i], tris);
  }

  template <typename M>
  typename COctreeSMC<M>::OctreeNode *COctreeSMC<M>::find_leaf(OctreeNode *node, const CPoint &p) const
  {
    if (node == NULL)
      return NULL;
    if (p[0] < node->min[0] || p[0] > node->max[0] ||
        p[1] < node->min[1] || p[1] > node->max[1] ||
        p[2] < node->min[2] || p[2] > node->max[2])
      return NULL;
    if (node->is_leaf())
      return node;
    CPoint mid = (node->min + node->max) / 2.0;
    int index = 0;
    if (p[0] >= mid[0])
      index |= 1;
    if (p[1] >= mid[1])
      index |= 2;
    if (p[2] >= mid[2])
      index |= 4;
    return find_leaf(node->children[index], p);
  }

  template <typename M>
  void COctreeSMC<M>::insert_face(OctreeNode *node, typename M::CFace *face, const CPoint &centroid)
  {
    if (node->depth >= m_maxDepth)
    {
      node->faces.push_back(face);
      return;
    }

    CPoint mid = (node->min + node->max) / 2.0;
    int index = 0;
    if (centroid[0] >= mid[0])
      index |= 1;
    if (centroid[1] >= mid[1])
      index |= 2;
    if (centroid[2] >= mid[2])
      index |= 4;

    if (node->children[index] == NULL)
    {
      CPoint cmin = node->min;
      CPoint cmax = node->max;
      if (index & 1)
        cmin[0] = mid[0];
      else
        cmax[0] = mid[0];
      if (index & 2)
        cmin[1] = mid[1];
      else
        cmax[1] = mid[1];
      if (index & 4)
        cmin[2] = mid[2];
      else
        cmax[2] = mid[2];

      node->children[index] = new OctreeNode(cmin, cmax, node->depth + 1);
    }

    insert_face(node->children[index], face, centroid);
  }

  template <typename M>
  void COctreeSMC<M>::collect_leaves(OctreeNode *node, vector<OctreeNode *> &leaves)
  {
    if (node == NULL)
      return;
    if (node->is_leaf())
    {
      leaves.push_back(node);
      return;
    }
    for (int i = 0; i < 8; ++i)
      collect_leaves(node->children[i], leaves);
  }

  template <typename M>
  CPoint COctreeSMC<M>::face_centroid(typename M::CFace *face) const
  {
    CPoint c(0, 0, 0);
    int count = 0;
    for (typename M::FaceVertexIterator fv(face); !fv.end(); fv++)
    {
      c += fv.value()->point();
      ++count;
    }
    if (count > 0)
      c /= static_cast<double>(count);
    return c;
  }

  template <typename M>
  void COctreeSMC<M>::face_vertices(typename M::CFace *face, vector<typename M::CVertex *> &verts) const
  {
    verts.clear();
    for (typename M::FaceVertexIterator fv(face); !fv.end(); fv++)
    {
      verts.push_back(fv.value());
    }
  }

  template <typename M>
  bool COctreeSMC<M>::are_coplanar(const vector<typename M::CVertex *> &a, const vector<typename M::CVertex *> &b) const
  {
    if (a.size() != 3 || b.size() != 3)
      return false;
    CPoint n1 = (a[1]->point() - a[0]->point()) ^ (a[2]->point() - a[0]->point());
    CPoint n2 = (b[1]->point() - b[0]->point()) ^ (b[2]->point() - b[0]->point());
    double l1 = n1.norm();
    double l2 = n2.norm();
    if (l1 <= 0 || l2 <= 0)
      return false;
    n1 /= l1;
    n2 /= l2;
    if (fabs(n1 * n2) < 0.999)
      return false;
    double d1 = n1 * a[0]->point();
    double d2 = n1 * b[0]->point();
    return fabs(d1 - d2) < 1e-6;
  }

  template <typename M>
  bool COctreeSMC<M>::build_quad(const vector<typename M::CVertex *> &a, const vector<typename M::CVertex *> &b,
                                 vector<typename M::CVertex *> &quad) const
  {
    if (!are_coplanar(a, b))
      return false;

    vector<typename M::CVertex *> uniq;
    uniq.reserve(4);
    for (size_t i = 0; i < a.size(); ++i)
    {
      bool found = false;
      for (size_t k = 0; k < uniq.size(); ++k)
      {
        if (uniq[k] == a[i])
        {
          found = true;
          break;
        }
      }
      if (!found)
        uniq.push_back(a[i]);
    }
    for (size_t i = 0; i < b.size(); ++i)
    {
      bool found = false;
      for (size_t k = 0; k < uniq.size(); ++k)
      {
        if (uniq[k] == b[i])
        {
          found = true;
          break;
        }
      }
      if (!found)
        uniq.push_back(b[i]);
    }

    if (uniq.size() != 4)
      return false;

    CPoint normal = (a[1]->point() - a[0]->point()) ^ (a[2]->point() - a[0]->point());
    double nlen = normal.norm();
    if (nlen <= 0)
      return false;
    normal /= nlen;

    typename M::CVertex *s0 = NULL;
    typename M::CVertex *s1 = NULL;
    for (size_t i = 0; i < a.size(); ++i)
    {
      for (size_t j = 0; j < b.size(); ++j)
      {
        if (a[i] == b[j])
        {
          if (s0 == NULL)
            s0 = a[i];
          else if (s1 == NULL && a[i] != s0)
            s1 = a[i];
        }
      }
    }
    if (s0 == NULL || s1 == NULL)
      return false;

    typename M::CVertex *o1 = NULL;
    typename M::CVertex *o2 = NULL;
    for (size_t i = 0; i < a.size(); ++i)
    {
      if (a[i] != s0 && a[i] != s1)
        o1 = a[i];
    }
    for (size_t i = 0; i < b.size(); ++i)
    {
      if (b[i] != s0 && b[i] != s1)
        o2 = b[i];
    }
    if (o1 == NULL || o2 == NULL)
      return false;

    vector<typename M::CVertex *> cand1;
    cand1.push_back(o1);
    cand1.push_back(s0);
    cand1.push_back(o2);
    cand1.push_back(s1);

    vector<typename M::CVertex *> cand2;
    cand2.push_back(o1);
    cand2.push_back(s1);
    cand2.push_back(o2);
    cand2.push_back(s0);

    auto accept_candidate = [&](vector<typename M::CVertex *> &cand) -> bool
    {
      if (valid_quad(cand, normal))
      {
        quad = cand;
        return true;
      }
      reverse(cand.begin(), cand.end());
      if (valid_quad(cand, normal))
      {
        quad = cand;
        return true;
      }
      return false;
    };

    if (accept_candidate(cand1))
      return true;
    if (accept_candidate(cand2))
      return true;

    return false;
  }

  template <typename M>
  bool COctreeSMC<M>::valid_triangle(const vector<typename M::CVertex *> &verts) const
  {
    if (verts.size() != 3)
      return false;
    for (size_t i = 0; i < verts.size(); ++i)
    {
      for (size_t j = i + 1; j < verts.size(); ++j)
      {
        if (verts[i] == verts[j])
          return false;
      }
    }
    CPoint n = (verts[1]->point() - verts[0]->point()) ^ (verts[2]->point() - verts[0]->point());
    return n.norm() > 1e-8;
  }

  template <typename M>
  bool COctreeSMC<M>::valid_quad(const vector<typename M::CVertex *> &verts, const CPoint &normal) const
  {
    if (verts.size() != 4)
      return false;
    for (size_t i = 0; i < verts.size(); ++i)
    {
      for (size_t j = i + 1; j < verts.size(); ++j)
      {
        if (verts[i] == verts[j])
          return false;
      }
    }

    CPoint n = normal;
    double nlen = n.norm();
    if (nlen <= 0)
      return false;
    n /= nlen;
    double d = n * verts[0]->point();
    for (size_t i = 1; i < verts.size(); ++i)
    {
      double dist = fabs(n * verts[i]->point() - d);
      if (dist > 1e-6)
        return false;
    }

    CPoint n0 = (verts[1]->point() - verts[0]->point()) ^ (verts[2]->point() - verts[0]->point());
    CPoint n1 = (verts[2]->point() - verts[0]->point()) ^ (verts[3]->point() - verts[0]->point());
    return n0.norm() > 1e-8 && n1.norm() > 1e-8;
  }

  template <typename M>
  typename M::CVertex *COctreeSMC<M>::get_or_create_vertex(M *out, typename M::CVertex *v,
                                                           map<typename M::CVertex *, typename M::CVertex *> &vmap, int &vid) const
  {
    typename M::CVertex *&cached = vmap[v];
    if (cached == NULL)
    {
      cached = out->createVertex(vid++);
      cached->point() = v->point();
    }
    return cached;
  }

  template <typename M>
  M *COctreeSMC<M>::quad_mesh()
  {
    M *out = new M();
    int vid = 1;
    int fid = 1;

    m_tris.clear();
    build_triangles(m_tris);
    vector<int> all_ids;
    all_ids.reserve(m_tris.size());
    for (size_t i = 0; i < m_tris.size(); ++i)
      all_ids.push_back(static_cast<int>(i));

    const int maxTrisPerNode = 80;
    build_octree(m_root, all_ids, m_maxDepth, maxTrisPerNode);
    classify_leaves(m_root, m_tris);

    const double base = (m_root->max[0] - m_root->min[0]) / static_cast<double>(1 << m_maxDepth);
    const double eps = base * 0.25;
    const double corner_offset = 0.1;

    map<VertKey, typename M::CVertex *> vmap;
    map<EdgeKey, int> edge_use;
    map<EdgeKey, int> dir_edge_use;
    auto to_index = [&](double v, int axis) -> int
    {
      double t = (v - m_root->min[axis]) / base;
      return static_cast<int>(floor(t + 0.5));
    };
    auto make_key = [&](const CPoint &p) -> VertKey
    {
      return {to_index(p[0], 0), to_index(p[1], 1), to_index(p[2], 2)};
    };
    auto get_vertex = [&](const CPoint &p) -> typename M::CVertex *
    {
      VertKey key = make_key(p);
      typename M::CVertex *&v = vmap[key];
      if (v == NULL)
      {
        v = out->createVertex(vid++);
        v->point() = p;
      }
      return v;
    };
    auto can_add_face = [&](vector<typename M::CVertex *> &verts) -> bool
    {
      size_t n = verts.size();
      for (size_t i = 0; i < n; ++i)
      {
        int a = verts[i]->id();
        int b = verts[(i + 1) % n]->id();
        if (a == b)
          return false;
        EdgeKey key;
        key.a = a < b ? a : b;
        key.b = a < b ? b : a;
        if (edge_use[key] >= 2)
          return false;
      }
      bool need_flip = false;
      for (size_t i = 0; i < n; ++i)
      {
        int a = verts[i]->id();
        int b = verts[(i + 1) % n]->id();
        EdgeKey dkey{a, b};
        if (dir_edge_use[dkey] > 0)
        {
          need_flip = true;
          break;
        }
      }
      if (need_flip)
      {
        reverse(verts.begin() + 1, verts.end());
      }
      for (size_t i = 0; i < n; ++i)
      {
        int a = verts[i]->id();
        int b = verts[(i + 1) % n]->id();
        EdgeKey dkey{a, b};
        if (dir_edge_use[dkey] > 0)
          return false;
      }
      for (size_t i = 0; i < n; ++i)
      {
        int a = verts[i]->id();
        int b = verts[(i + 1) % n]->id();
        EdgeKey key;
        key.a = a < b ? a : b;
        key.b = a < b ? b : a;
        edge_use[key] += 1;
        EdgeKey dkey{a, b};
        dir_edge_use[dkey] += 1;
      }
      return true;
    };

    vector<OctreeNode *> leaves;
    collect_leaves(m_root, leaves);
    auto add_quad_face = [&](const CPoint &p0, const CPoint &p1, const CPoint &p2, const CPoint &p3)
    {
      vector<typename M::CVertex *> quad;
      quad.push_back(get_vertex(p0));
      quad.push_back(get_vertex(p1));
      quad.push_back(get_vertex(p2));
      quad.push_back(get_vertex(p3));
      if (can_add_face(quad))
        out->createFace(quad, fid++);
    };
    auto add_tri_face = [&](const CPoint &p0, const CPoint &p1, const CPoint &p2)
    {
      vector<typename M::CVertex *> tri;
      tri.push_back(get_vertex(p0));
      tri.push_back(get_vertex(p1));
      tri.push_back(get_vertex(p2));
      if (can_add_face(tri))
        out->createFace(tri, fid++);
    };
    auto mc_cut_triangles = [&](int cfg, const CPoint corners[8], const CPoint &cmin, const CPoint &cmax, bool centerInside)
    {
      if (cfg == 0 || cfg == 255)
        return;
      CPoint edgePts[12];
      for (int e = 0; e < 12; ++e)
      {
        int a = kEdgeCorners[e][0];
        int b = kEdgeCorners[e][1];
        CPoint pa = corners[a];
        CPoint pb = corners[b];
        edgePts[e] = (pa + pb) * 0.5;
      }
      CPoint cellCenter = (cmin + cmax) / 2.0;
      for (int i = 0; kTriTable[cfg][i] != -1; i += 3)
      {
        int e0 = kTriTable[cfg][i];
        int e1 = kTriTable[cfg][i + 1];
        int e2 = kTriTable[cfg][i + 2];
        CPoint p0 = edgePts[e0];
        CPoint p1 = edgePts[e1];
        CPoint p2 = edgePts[e2];
        CPoint n = (p1 - p0) ^ (p2 - p0);
        if (n.norm() <= 1e-8)
          continue;
        CPoint triCenter = (p0 + p1 + p2) / 3.0;
        double dir = n * (triCenter - cellCenter);
        if ((centerInside && dir < 0) || (!centerInside && dir > 0))
        {
          CPoint tmp = p1;
          p1 = p2;
          p2 = tmp;
        }
        add_tri_face(p0, p1, p2);
      }
    };

    map<OctreeNode *, int> cfg_map;
    for (size_t i = 0; i < leaves.size(); ++i)
    {
      OctreeNode *leaf = leaves[i];
      CPoint cmin = leaf->min;
      CPoint cmax = leaf->max;
      CPoint center = (cmin + cmax) / 2.0;
      int cfg = 0;
      for (int k = 0; k < 8; ++k)
      {
        CPoint corner(
            kCornerOffset[k][0] ? cmax[0] : cmin[0],
            kCornerOffset[k][1] ? cmax[1] : cmin[1],
            kCornerOffset[k][2] ? cmax[2] : cmin[2]);
        CPoint sample = corner + (center - corner) * corner_offset;
        if (point_in_mesh(sample, m_tris))
          cfg |= (1 << k);
      }
      cfg_map[leaf] = cfg;
    }

    for (size_t i = 0; i < leaves.size(); ++i)
    {
      OctreeNode *leaf = leaves[i];
      CPoint cmin = leaf->min;
      CPoint cmax = leaf->max;
      CPoint center = (cmin + cmax) / 2.0;
      int cfg = cfg_map[leaf];
      if (cfg == 0)
        continue;

      CPoint corners[8];
      for (int k = 0; k < 8; ++k)
      {
        corners[k] = CPoint(
            kCornerOffset[k][0] ? cmax[0] : cmin[0],
            kCornerOffset[k][1] ? cmax[1] : cmin[1],
            kCornerOffset[k][2] ? cmax[2] : cmin[2]);
      }

      if (cfg == 255)
      {
        CPoint p;
        OctreeNode *nb;

        p = CPoint(cmin[0] - eps, center[1], center[2]);
        nb = find_leaf(m_root, p);
        if (nb == NULL || cfg_map[nb] != 255)
          add_quad_face(corners[0], corners[4], corners[7], corners[3]);

        p = CPoint(cmax[0] + eps, center[1], center[2]);
        nb = find_leaf(m_root, p);
        if (nb == NULL || cfg_map[nb] != 255)
          add_quad_face(corners[1], corners[2], corners[6], corners[5]);

        p = CPoint(center[0], cmin[1] - eps, center[2]);
        nb = find_leaf(m_root, p);
        if (nb == NULL || cfg_map[nb] != 255)
          add_quad_face(corners[0], corners[1], corners[5], corners[4]);

        p = CPoint(center[0], cmax[1] + eps, center[2]);
        nb = find_leaf(m_root, p);
        if (nb == NULL || cfg_map[nb] != 255)
          add_quad_face(corners[3], corners[7], corners[6], corners[2]);

        p = CPoint(center[0], center[1], cmin[2] - eps);
        nb = find_leaf(m_root, p);
        if (nb == NULL || cfg_map[nb] != 255)
          add_quad_face(corners[0], corners[3], corners[2], corners[1]);

        p = CPoint(center[0], center[1], cmax[2] + eps);
        nb = find_leaf(m_root, p);
        if (nb == NULL || cfg_map[nb] != 255)
          add_quad_face(corners[4], corners[5], corners[6], corners[7]);
      }
      else
      {
        bool centerInside = point_in_mesh_vote(center, m_tris);
        mc_cut_triangles(cfg, corners, cmin, cmax, centerInside);
      }
    }

    return out;
  }
}

#endif
