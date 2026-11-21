#include "MarchingCubesVoxelMesh.h"
#include "Math/IndexConvert.h"
#include "VoxelMaterials.h"
#include "Math/Noise.h"
#include "System/BitOps.h"
#include <GL/glew.h>



/*
			
*/

int edgeTable[256] = {
0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0 };
int triTable[256][16] =
{ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
{3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
{4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
{9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
{10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
{5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
{8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
{2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
{11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
{5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
{11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
{11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
{9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
{6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
{6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
{8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
{7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
{3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
{9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
{8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
{0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
{6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
{10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
{10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
{0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
{3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
{9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
{8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
{3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
{10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
{10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
{7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
{1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
{11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
{8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
{0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
{7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
{7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
{10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
{0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
{7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
{9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
{6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
{4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
{10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
{8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
{1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
{10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
{10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
{9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
{7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
{3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
{7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
{3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
{6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
{9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
{1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
{4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
{7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
{6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
{0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
{6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
{0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
{11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
{6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
{5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
{9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
{1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
{10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
{0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
{5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
{11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
{9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
{7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
{2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
{9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
{1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
{10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
{2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
{0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
{0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
{9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
{5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
{5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
{9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
{1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
{3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
{4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
{9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
{11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
{2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
{9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
{3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
{1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
{4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
{0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
{1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1} };



glm::vec3 grid[8] =
{
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(1.0f, 0.0f, 0.0f),
	glm::vec3(1.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 1.0f),
	glm::vec3(1.0f, 0.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, 1.0f),
	glm::vec3(0.0f, 1.0f, 1.0f)
};

glm::ivec3 gridI[8] =
{
	glm::ivec3(0, 0, 0),
	glm::ivec3(1, 0, 0),
	glm::ivec3(1, 1, 0),
	glm::ivec3(0, 1, 0),
	glm::ivec3(0, 0, 1),
	glm::ivec3(1, 0, 1),
	glm::ivec3(1, 1, 1),
	glm::ivec3(0, 1, 1)
};

glm::vec3 cubePositions[] = 
{
	{0.0f, 0.0f, 0.0f}, //0
	{1.0f, 0.0f, 0.0f}, //1
	{0.0f, 1.0f, 0.0f}, //2
	{1.0f, 1.0f, 0.0f}, //3
	{0.0f, 0.0f, 1.0f}, //4
	{1.0f, 0.0f, 1.0f}, //5
	{0.0f, 1.0f, 1.0f}, //6
	{1.0f, 1.0f, 1.0f}  //7
};

unsigned int cubeIndices[] = {
	// --- x = 0 face (vertices 0,2,6,4) ---
	6, 2, 0,
	4, 6, 0,

	// --- x = 1 face (vertices 1,5,7,3) ---
	7, 5, 1,
	3, 7, 1,

	// --- y = 0 face (vertices 0,1,5,4) ---
	0, 1, 5,
	0, 5, 4,

	// --- y = 1 face (vertices 2,6,7,3) ---
	2, 6, 7,
	2, 7, 3,

	// --- z = 0 face (vertices 0,1,3,2) ---
	3, 1, 0,
	2, 3, 0,

	// --- z = 1 face (vertices 4,5,7,6) ---
	4, 5, 7,
	4, 7, 6
};

void packCellDataTriangles(MarchingCubesVoxelMesh::CellData& cell, const int* triangles) 
{
	//triangle index range: [-1; (2^18)-2)
	const unsigned long long bitsPerIndex = 18ULL;
	cell.data[0] = 0ULL;
	cell.data[1] &= ~((1ULL << 26ULL) - 1ULL);
	cell.data[0] |= static_cast<unsigned long long>(triangles[0] + 1) << (bitsPerIndex * 0ULL);
	cell.data[0] |= static_cast<unsigned long long>(triangles[1] + 1) << (bitsPerIndex * 1ULL);
	cell.data[0] |= static_cast<unsigned long long>(triangles[2] + 1) << (bitsPerIndex * 2ULL);
	cell.data[0] |= static_cast<unsigned long long>(triangles[3] + 1) << (bitsPerIndex * 3ULL);
	cell.data[1] &= ~((1ULL << 26ULL) - 1);
	cell.data[1] |= static_cast<unsigned long long>(triangles[3] + 1) >> (bitsPerIndex - 8ULL);
	cell.data[1] |= static_cast<unsigned long long>(triangles[4] + 1) << (8ULL);
}

void unpackCellDataTriangles(MarchingCubesVoxelMesh::CellData& cell, int* triangles) 
{
	const unsigned long long bitsPerIndex = 18ULL;
	const unsigned long long mask = (1ULL << bitsPerIndex) - 1ULL;
	const unsigned long long mask8 = (1ULL << 8ULL) - 1ULL;
	triangles[0] = int(((cell.data[0] >> (bitsPerIndex * 0ULL)) & mask)) - 1;
	triangles[1] = int(((cell.data[0] >> (bitsPerIndex * 1ULL)) & mask)) - 1;
	triangles[2] = int(((cell.data[0] >> (bitsPerIndex * 2ULL)) & mask)) - 1;
	triangles[3] = int(((cell.data[0] >> (bitsPerIndex * 3ULL)) & mask)) - 1;
	triangles[3] += (cell.data[1] & mask8) << 10ULL;
	triangles[4] = int((cell.data[1] >> 8ULL) & mask) - 1;
}

void updateCellTriangle(MarchingCubesVoxelMesh::CellData& cell, int triangleCellIndex, int movedTriangleIndex) 
{
	const unsigned long long bitsPerIndex = 18ULL;
	const unsigned long long mask = (1ULL << bitsPerIndex) - 1ULL;
	if(triangleCellIndex == 0) 
	{
		cell.data[0] &= ~(mask);
		cell.data[0] |= static_cast<unsigned long long>(movedTriangleIndex + 1) << (bitsPerIndex * 0ULL);
		return;
	}
	if(triangleCellIndex == 1) 
	{
		cell.data[0] &= ~(mask << bitsPerIndex);
		cell.data[0] |= static_cast<unsigned long long>(movedTriangleIndex + 1) << (bitsPerIndex * 1ULL);
		return;
	}
	if(triangleCellIndex == 2) 
	{
		cell.data[0] &= ~(mask << (bitsPerIndex * 2ULL));
		cell.data[0] |= static_cast<unsigned long long>(movedTriangleIndex + 1) << (bitsPerIndex * 2ULL);
		return;
	}
	if(triangleCellIndex == 3) 
	{
		cell.data[0] &= ~(mask << (bitsPerIndex * 3ULL));
		cell.data[0] |= static_cast<unsigned long long>(movedTriangleIndex + 1) << (bitsPerIndex * 3ULL);
		cell.data[1] &= ~(mask >> 10ULL);
		cell.data[1] |= static_cast<unsigned long long>(movedTriangleIndex + 1) >> (10ULL);
		return;
	}
	if(triangleCellIndex == 4) 
	{
		cell.data[1] &= ~(mask << 8ULL);
		cell.data[1] |= static_cast<unsigned long long>(movedTriangleIndex + 1) << (8ULL);
	}
}

void flagCubeIndex(MarchingCubesVoxelMesh::CellData& cell, int flagIndex, bool active) 
{
	if(active) 
	{
		cell.data[1] |= (1ULL << flagIndex) << (56ULL);
	}
	else 
	{
		cell.data[1] &= ~((1ULL << flagIndex) << 56ULL);
	}
}

inline unsigned char EdgeIntersection(signed char valp1, signed char valp2)
{
	if (valp1 == valp2)
        return 0;

    return (valp1 * 255) / (valp1 - valp2);
}

unsigned int packMarchingCubesVertice(unsigned int edgeX, unsigned int edgeY, unsigned int axis, float offset) 
{
	unsigned int ioffset = offset * float(1 << 18);
	if(ioffset >= (1 << 18)) ioffset = (1 << 18) - 1;
	return edgeX + (edgeY << 6) + (axis << 12) + (ioffset << 14);
}

unsigned int packMaterialOffset(glm::ivec3 materialIndexes) 
{
	return materialIndexes.x + (materialIndexes.y << 10) + (materialIndexes.z << 20);
}

unsigned char getCellCubeIndex(const MarchingCubesVoxelMesh::CellData& cell) 
{
	return cell.data[1] >> 56ULL;
}

glm::vec2 octWrap(glm::vec2 v)
{
    return (1.0f - glm::abs(glm::vec2(v.y, v.x))) * (glm::sign(v));
}
 
inline unsigned int packNormal(const glm::vec3& n)
{
    const unsigned int x = uint32_t((n.x * 0.5f + 0.5f) * 1024.0f);
    const unsigned int y = uint32_t((n.y * 0.5f + 0.5f) * 1024.0f);
    const unsigned int z = uint32_t((n.z * 0.5f + 0.5f) * 1024.0f);

    return x | (y << 10) | (z << 20);
}
 
glm::vec3 decodeNormal(glm::vec2 f)
{
    f = f * 2.0f - 1.0f;
    glm::vec3 n = glm::vec3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = glm::max(-n.z, 0.0f);
    n.x += n.x >= 0.0 ? -t : t;
    n.y += n.y >= 0.0 ? -t : t;
    return normalize(n);
}


MarchingCubesVoxelMesh::MarchingCubesVoxelMesh(VoxelData* data, float threshold, unsigned int levelofdetails, glm::vec3 offset) :
cells()
{
	this->data = data;
	this->offset = offset;
	this->threshold = threshold;
	this->levelofdetails = levelofdetails;
	dimensions = data->getDimensions();
	this->mesh.boundingBox = {0,0,0,0,0,0};
	this->mesh.indexCount = 0;
	this->mesh.indexOffset = 0;
	this->mesh.vao = nullptr;
	this->mesh.vertexOffset = 0;
	unsigned int edgesCount = 
	((dimensions.x + 1) * (dimensions.y + 1) * (dimensions.z)) +
	((dimensions.x + 1) * (dimensions.y) * (dimensions.z + 1)) + 
	((dimensions.x) * (dimensions.y + 1) * (dimensions.z + 1));
	unsigned int cellsCount = dimensions.x * dimensions.y * dimensions.z;
	unsigned int voxelsCount = (dimensions.x + 1) * (dimensions.y + 1) * (dimensions.z + 1);
	this->cells.init(dimensions.x * dimensions.y * dimensions.z);
	const int maskElementSize = sizeof(unsigned long long) * 8;
	const int cellUpdateMaskSize = cellsCount / maskElementSize + ((cellsCount % maskElementSize) ? 1 : 0);
	const int voxelUpdateMaskSize = voxelsCount / maskElementSize + ((voxelsCount % maskElementSize) ? 1 : 0);
	this->cellUpdateMask.init(cellUpdateMaskSize);
	this->voxelUpdateMask.init(voxelUpdateMaskSize);
	this->edges.init(edgesCount);
	this->cellUpdateQueueHead = 0;
	this->voxelUpdateQueueHead = 0;
	this->randomSeed = rand();

	CellData testCell;
	testCell.data[0] = testCell.data[1] = 0ULL;
	int testTriangles[5] = {-1, -1, -1, -1, -1};
	packCellDataTriangles(testCell, testTriangles);
	unpackCellDataTriangles(testCell, testTriangles);
	if(testTriangles[0] != -1 || testTriangles[1] != -1 || testTriangles[2] != -1 || testTriangles[3] != -1 || testTriangles[4] != -1) 
	{
		throw "Unpack fail";
	}
	
	for(int i = 0; i < 5; i++) 
	{
		testTriangles[i] = i;
	}
	assert(sizeof(unsigned long long) == 8);
	packCellDataTriangles(testCell, testTriangles);
	unpackCellDataTriangles(testCell, testTriangles);
	for(int i = 0; i < 5; i++) 
	{
		if(testTriangles[i] != i) throw "Pack or unpack fail";
	}
	for(int i = 0; i < 5; i++) 
	{
		testTriangles[i] = -1;
	}
	packCellDataTriangles(testCell, testTriangles);
	updateCellTriangle(testCell, 0, 0);
	updateCellTriangle(testCell, 1, 1);
	updateCellTriangle(testCell, 2, 2);
	updateCellTriangle(testCell, 3, 3);
	updateCellTriangle(testCell, 4, 4);
	unpackCellDataTriangles(testCell, testTriangles);
	for(int i = 0; i < 5; i++) 
	{
		if(testTriangles[i] != i) throw "Update fail";
	}
}

void MarchingCubesVoxelMesh::updateMesh()
{
	if(!allDirty) 
	{
		compactVertices.clear();
		indices.clear();
		for(int i = 0; i < voxelUpdateQueueHead; i++) 
		{		
			updateVoxel(a1to3ind(voxelUpdateQueue[i], dimensions.y + 1, dimensions.z + 1));
		}
		for(int i = 0; i < cellUpdateQueueHead; i++) 
		{		
			updateCell(a1to3ind(cellUpdateQueue[i], dimensions.y, dimensions.z));
		}
		cellUpdateQueueHead = 0;
		voxelUpdateQueueHead = 0;
		memset(cellUpdateMask.data(), 0, cellUpdateMask.size() * sizeof(unsigned long long));
		memset(voxelUpdateMask.data(), 0, voxelUpdateMask.size() * sizeof(unsigned long long));
		this->updateIndices();
		return;
	}
	clear();
	const int lineMass = dimensions.z;
	const int sliceMass = dimensions.y * dimensions.z;
	const int iStride = 1;
	for (int i = 0; i < dimensions.x + 1; i += iStride)
	{
		bool skipSlice = false;
		if (i + 1 < dimensions.x) 
		{
			int m0 = data->getXSliceMass(i);
			int m1 = data->getXSliceMass(i + 1);

			if ((m0 == 0 && m1 == 0) || (m0 == sliceMass && m1 == sliceMass)) 
			{
				skipSlice = true;
			}
		}
		const int jStride = 1;
		for (int j = 0; j < dimensions.y + 1; j += jStride)
		{
			bool skipLine = false;
			if ((i + 1 < dimensions.x) && (j + 1 < dimensions.y)) {
				int m00 = data->getXYLineMass(i, j);
				int m10 = data->getXYLineMass(i + 1, j);
				int m01 = data->getXYLineMass(i, j + 1);
				int m11 = data->getXYLineMass(i + 1, j + 1);

				if ((m00 == 0 && m01 == 0 && m10 == 0 && m11 == 0) || (m00 == lineMass && m01 == lineMass && m10 == lineMass && m11 == lineMass)) 
				{
					skipLine = true;
				}
			}
			int kStride = 1;
			for (int k = 0; k < dimensions.z + 1; k += kStride)
			{
				updateVoxel(glm::ivec3(i, j, k));
			}
		}
	}

	for (int i = 0; i < dimensions.x; i += iStride)
	{
		bool skipSlice = false;
		if (i + 1 < dimensions.x) 
		{
			int m0 = data->getXSliceMass(i);
			int m1 = data->getXSliceMass(i + 1);

			if ((m0 == 0 && m1 == 0) || (m0 == sliceMass && m1 == sliceMass)) 
			{
				skipSlice = true;
			}
		}
		const int jStride = skipSlice ? (dimensions.y - 1) : 1;
		for (int j = 0; j < dimensions.y; j += jStride)
		{
			bool skipLine = false;
			if ((i + 1 < dimensions.x) && (j + 1 < dimensions.y)) {
				int m00 = data->getXYLineMass(i, j);
				int m10 = data->getXYLineMass(i + 1, j);
				int m01 = data->getXYLineMass(i, j + 1);
				int m11 = data->getXYLineMass(i + 1, j + 1);

				if ((m00 == 0 && m01 == 0 && m10 == 0 && m11 == 0) || (m00 == lineMass && m01 == lineMass && m10 == lineMass && m11 == lineMass)) 
				{
					skipLine = true;
				}
			}
			int kStride = skipLine ? (dimensions.z - 1) : 1;
			for (int k = 0; k < dimensions.z; k += kStride)
			{
				updateCell(glm::ivec3(i, j, k));
			}
		}
	}
	this->updateIndices();
	memset(cellUpdateMask.data(), 0, cellUpdateMask.size() * sizeof(unsigned long long));
	memset(voxelUpdateMask.data(), 0, voxelUpdateMask.size() * sizeof(unsigned long long));
	allDirty = false;
}

void MarchingCubesVoxelMesh::updateCell(glm::ivec3 index) 
{
	CellData& thisCell = cells[a3to1ind(index, dimensions.y, dimensions.z)];
	int zAxisList[12] = {0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2};
	int xAxisList[12] = {1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};
	int yAxisList[12] = {2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1};
	glm::ivec2 edgeOffsets[12] = {{0, 0}, {1, 0}, {1, 0}, {0, 0}, {0, 1}, {1, 1}, {1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 1}};
	unsigned int edgeVertexIndexList[12];
	int edgesPerAxis[3] = {
		(dimensions.x + 1) * (dimensions.y + 1) * (dimensions.z),
		(dimensions.x + 1) * (dimensions.y) * (dimensions.z + 1),
		(dimensions.x) * (dimensions.y + 1) * (dimensions.z + 1)	
	};
	//Generate tri table to vertex mapping
	for(int edge = 0; edge < 12; edge++) 
	{
		int axisOffset = 0;
		for(int aCount = 0; aCount < zAxisList[edge]; aCount++) 
		{
			axisOffset += edgesPerAxis[aCount];
		}
		glm::ivec3 indexOffset;
		glm::ivec3 axisAlignedIndex;
		axisAlignedIndex[0] = index[xAxisList[edge]] + edgeOffsets[edge][0];
		axisAlignedIndex[1] = index[yAxisList[edge]] + edgeOffsets[edge][1];
		axisAlignedIndex[2] = index[zAxisList[edge]];
		glm::ivec3 axisAlignedDimensions = glm::ivec3(
			dimensions[xAxisList[edge]],
			dimensions[yAxisList[edge]],
			dimensions[zAxisList[edge]]
		);
		edgeVertexIndexList[edge] = a3to1ind(axisAlignedIndex, axisAlignedDimensions.y + 1 /*Height*/, axisAlignedDimensions.z /*Length*/) + axisOffset;
		if(edgeVertexIndexList[edge] >= edges.size()) throw std::logic_error("Edge index out of edge vertices bounds");
	}
	unsigned char cubeindex = getCellCubeIndex(thisCell);
	unsigned int triangleEdgeIndices[3];
	unsigned int triangleMaterials[3];
	Triangle triangles[5];
	triangles[0].indicesAndDecorations = 0;
	triangles[1].indicesAndDecorations = 0;
	triangles[2].indicesAndDecorations = 0;
	triangles[3].indicesAndDecorations = 0;
	triangles[4].indicesAndDecorations = 0;
	unsigned char triangleCount = 0;
	unsigned char decorationId = (unsigned char)DecorationType::None;
	removeTriangles(thisCell, a3to1ind(index, dimensions.y, dimensions.z));
	for (int l = 0; triTable[cubeindex][l] != -1; l++) {
		int triTableIndex = triTable[cubeindex][l];
		unsigned long long edgeVertexIndex = edgeVertexIndexList[triTableIndex];
		EdgeInfo& edge = edges[edgeVertexIndex];
		const int xAxis = xAxisList[triTableIndex];
		const int yAxis = yAxisList[triTableIndex];
		const int zAxis = zAxisList[triTableIndex];
		const int edgeX = index[xAxis] + edgeOffsets[triTableIndex][0];
		const int edgeY = index[yAxis] + edgeOffsets[triTableIndex][1];
		triangles[triangleCount].indicesAndDecorations |= (edgeVertexIndex << (18ULL * (l % 3)));
		if((triangles[triangleCount].indicesAndDecorations & (((1ULL << 10ULL) - 1ULL) << 54ULL)) == 0) 
		{
			triangles[triangleCount].indicesAndDecorations |= static_cast<unsigned long long>(VoxelMaterials::getInstance().getById(edge.materialId).decorations) << 54ULL;
			if(!VoxelMaterials::getInstance().getById(edge.materialId).smooth) 
			{
				triangles[triangleCount].indicesAndDecorations |= 1ULL << 61ULL;
			}
		}
		triangleMaterials[l % 3] = VoxelMaterials::getInstance().getById(edge.materialId).texture;

		if (l % 3 == 2)
		{
			triangles[triangleCount].material = packMaterialOffset(glm::ivec3(
				triangleMaterials[0],
				triangleMaterials[1],
				triangleMaterials[2]
			));
			triangleCount++;
		}
	}
	addTriangles(triangles, triangleCount, thisCell, a3to1ind(index, dimensions.y, dimensions.z));
}

void MarchingCubesVoxelMesh::uploadMesh() 
{
	if(compactVertices.size() > 0) this->uploadData();
}

void MarchingCubesVoxelMesh::uploadData() 
{
	if(vao == 0) 
	{
		vao.create();
		grassVAO.create();
	}
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * compactVertices.size(), compactVertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
	vao.vertexAttribPointer(Kirino::VertexArrayObject::VertexAttribPointer(true, vbo, 0, 1, GL_UNSIGNED_INT, false, sizeof(unsigned int) * (3), (void*)(sizeof(unsigned int) * 0), 0));
	vao.vertexAttribPointer(Kirino::VertexArrayObject::VertexAttribPointer(true, vbo, 1, 1, GL_UNSIGNED_INT, false, sizeof(unsigned int) * (3), (void*)(sizeof(unsigned int) * 1), 0));
	vao.vertexAttribPointer(Kirino::VertexArrayObject::VertexAttribPointer(true, vbo, 2, 1, GL_UNSIGNED_INT, false, sizeof(unsigned int) * (3), (void*)(sizeof(unsigned int) * 2), 0));
	vao.updateAttribs();	
	mesh.vao = &vao;
	

	glBindVertexArray(grassVAO);
	glGenBuffers(1, &grassVBO);
	glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GrassVertexInfoPacked) * grassData.size(), grassData.data(), GL_DYNAMIC_DRAW);
	grassVAO.vertexAttribPointer(Kirino::VertexArrayObject::VertexAttribPointer(true, grassVBO, 0, 1, GL_UNSIGNED_INT, false, sizeof(GrassVertexInfoPacked), 0, 1));
	grassVAO.vertexAttribPointer(Kirino::VertexArrayObject::VertexAttribPointer(true, grassVBO, 1, 1, GL_UNSIGNED_SHORT, false, sizeof(GrassVertexInfoPacked), (void*)(sizeof(unsigned int)), 1));
	grassVAO.updateAttribs();
	this->mesh.boundingBox.minX = 0.0f;
	this->mesh.boundingBox.minY = 0.0f;
	this->mesh.boundingBox.minZ = 0.0f;
	this->mesh.boundingBox.maxX = 32.0f;
	this->mesh.boundingBox.maxY = 32.0f;
	this->mesh.boundingBox.maxZ = 32.0f;
	mesh.indexCount = indices.size();
	mesh.indexOffset = 0;
	mesh.vertexOffset = 0;
	grassDataSize = grassData.size();
}

void MarchingCubesVoxelMesh::clear() 
{
	memset(cellUpdateMask.data(), 0, cellUpdateMask.size() * sizeof(unsigned long long));
	memset(voxelUpdateMask.data(), 0, voxelUpdateMask.size() * sizeof(unsigned long long));
	memset(cells.data(), 0, sizeof(CellData) * cells.size());
	for(CellData& cell : cells) 
	{
		cell.data[0] = cell.data[1] = 0;
	}
	memset(edges.data(), 0, sizeof(EdgeInfo) * edges.size());
	compactVertices.clear();
	indices.clear();
	triangles.clear();
	grassData.clear();
	triangleIndexToCellTriangleIndex.clear();
	
	cellUpdateQueueHead = 0;
	voxelUpdateQueueHead = 0;
	grassDataSize = 0;
}

void MarchingCubesVoxelMesh::clearAll() 
{
	clear();
	mesh.indexCount = 0;
}

MarchingCubesVoxelMesh::~MarchingCubesVoxelMesh() 
{
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
}

void MarchingCubesVoxelMesh::addTriangles(MarchingCubesVoxelMesh::Triangle* addedTriangles, int trianglesCount, CellData& cell, unsigned int cellIndex) 
{
	int triangleIndexes[maxTrianglesPerCell];
	triangleIndexes[0] = triangleIndexes[1] = triangleIndexes[2] = triangleIndexes[3] = triangleIndexes[4] = -1;
	for(int i = 0; i < trianglesCount; i++) 
	{
		triangleIndexes[i] = triangles.size();
		triangles.push_back(addedTriangles[i]);
		triangleIndexToCellTriangleIndex.push_back(cellIndex * maxTrianglesPerCell + i);
	}
	packCellDataTriangles(cell, triangleIndexes);
}
void MarchingCubesVoxelMesh::removeTriangles(CellData& cell, unsigned int cellIndex) 
{
	int triangleIndexes[maxTrianglesPerCell];	
	unpackCellDataTriangles(cell, triangleIndexes);
	for(int t = 0; t < maxTrianglesPerCell && triangleIndexes[t] >= 0; t++) 
	{
		int removedTriangleIndex = triangleIndexes[t];
		int movedTriangleCellTriangleIndex = triangleIndexToCellTriangleIndex.back();
		int movedTriangleCellIndex = movedTriangleCellTriangleIndex / maxTrianglesPerCell;
		//copy last
		triangles[removedTriangleIndex] = triangles.back();
		//copy index pointing to triangle index stored in cell
		triangleIndexToCellTriangleIndex[removedTriangleIndex] = movedTriangleCellTriangleIndex;
		//remove both last
		triangles.pop_back();
		triangleIndexToCellTriangleIndex.pop_back();
		if(movedTriangleCellIndex == cellIndex) 
		{
			triangleIndexes[movedTriangleCellTriangleIndex % maxTrianglesPerCell] = removedTriangleIndex;
		}
		else 
		{
			updateCellTriangle(cells[movedTriangleCellIndex], movedTriangleCellTriangleIndex % maxTrianglesPerCell, removedTriangleIndex);
		}
	}
	for(int t = 0; t < maxTrianglesPerCell; t++) 
		triangleIndexes[t] = -1;
	packCellDataTriangles(cell, triangleIndexes);
}

glm::vec3 unpackVertexPosition(unsigned int position) 
{
	float edgeX = float(position & 63u);
	float edgeY = float((position >> 6u) & 63u);
	unsigned int axis  = (position >> 12u) & 3u;
	float foffset = float((position >> 14u) & 262143u) / 8191.0f;
	// Branchless axis selection
	glm::vec3 fpos = glm::vec3(edgeX, edgeY, edgeY);
	fpos = glm::mix(glm::vec3(foffset, edgeX, edgeY), fpos, float(axis != 0u));
	fpos = glm::mix(fpos, glm::vec3(edgeX, foffset, edgeY), float(axis == 1u));
	fpos = glm::mix(fpos, glm::vec3(edgeX, edgeY, foffset), float(axis == 2u));
	return fpos;
}


unsigned int packedPositionFromEdgeIndexAndOffset(unsigned int index, unsigned char offset, glm::vec3* outPosition = nullptr) 
{
	unsigned int axis = index / 34848;
	unsigned int axisLocalIndexFlat = index % (34848);
	glm::ivec3 axisLocalIndex = a1to3ind(axisLocalIndexFlat, 33, 32);
	if(outPosition) 
	{
		unsigned int axisX = axis ? 0 : 1;
		unsigned int axisY = axis == 2 ? 1 : 2;
		(*outPosition)[axisX] = axisLocalIndex.x;
		(*outPosition)[axisY] = axisLocalIndex.y;
		(*outPosition)[axis] = axisLocalIndex.z + float(offset) / 255.0f;
	}
	return packMarchingCubesVertice(axisLocalIndex.x, axisLocalIndex.y, axis, (float(offset) / 255.0f + float(axisLocalIndex.z)) / 32.0f);
}

void MarchingCubesVoxelMesh::updateIndices() 
{
	bool hasDecorationsNew = false;
	indices.clear();
	compactVertices.clear();
	grassData.clear();
	for(Triangle& tri : triangles) 
	{
		unsigned int unpackedIndices[3];
		unpackedIndices[0] = (tri.indicesAndDecorations & ((1ULL << 18ULL) - 1ULL));
		unpackedIndices[1] = ((tri.indicesAndDecorations >> 18ULL) & ((1ULL << 18ULL) - 1ULL));
		unpackedIndices[2] = ((tri.indicesAndDecorations >> 36ULL) & ((1ULL << 18ULL) - 1ULL));
		glm::vec3 vertexPosition1;
		glm::vec3 vertexPosition2;
		glm::vec3 vertexPosition3;
		Vertex v;
		indices.push_back(compactVertices.size() + 0);
		indices.push_back(compactVertices.size() + 1);
		indices.push_back(compactVertices.size() + 2);
		//decorationsId is 10 bits
		unsigned int decorationsId = (tri.indicesAndDecorations >> 54ULL);
		//decoration flags
		unsigned int decorationFlags = decorationsId >> 7ULL;
		decorationsId &= ~(0b111 << 7ULL);

		glm::vec3 dv1;
		glm::vec3 dv2;
		glm::vec3 flatNormal;
		bool isFlatSurface = decorationFlags & 0b001;
		bool needsFlatNormal = 
			decorationsId == (unsigned int)DecorationType::Grass ||
			isFlatSurface;

		unsigned int vertexPackedPosition1 = packedPositionFromEdgeIndexAndOffset(unpackedIndices[0], edges[unpackedIndices[0]].offset, &vertexPosition1);
		unsigned int vertexPackedPosition2 = packedPositionFromEdgeIndexAndOffset(unpackedIndices[1], edges[unpackedIndices[1]].offset, &vertexPosition2);
		unsigned int vertexPackedPosition3 = packedPositionFromEdgeIndexAndOffset(unpackedIndices[2], edges[unpackedIndices[2]].offset, &vertexPosition3);
		
		if(needsFlatNormal) 
		{
			dv1 = vertexPosition2 - vertexPosition3;
			dv2 = vertexPosition1 - vertexPosition3;
			flatNormal = glm::normalize(
				glm::cross(
					dv2,
					dv1
				)
			);
		}
		v.textureOffsets = tri.material;
		v.position = vertexPackedPosition1;
		glm::vec3 n1 = isFlatSurface ? flatNormal : data->getNormal(vertexPosition1);
		v.normal = packNormal(n1);
		compactVertices.push_back(v);
		
		v.position = vertexPackedPosition2;
		glm::vec3 n2 = isFlatSurface ? flatNormal : data->getNormal(vertexPosition2);
		v.normal = packNormal(n2);
		compactVertices.push_back(v);
		
		v.position = vertexPackedPosition3;
		glm::vec3 n3 = isFlatSurface ? flatNormal : data->getNormal(vertexPosition3);
		v.normal = packNormal(n3);
		compactVertices.push_back(v);
		
		
		
		if(decorationsId == (unsigned int)DecorationType::Grass) 
		{
			const unsigned int grassDensity = 2;
			for(int grassId = 0; grassId < grassDensity; grassId++) 
			{
				if(flatNormal.y > 0.8f) 
				{
					hasDecorationsNew = true;
					const float sizeBase = 0.18f;
					const float sizeGrowth = 0.11f;
					glm::ivec3 index = glm::ivec3(vertexPosition1);
					unsigned int grassSeed = Noise::uintGet(randomSeed + index.x, index.y + grassId, index.z);
					GrassVertexInfoUnpacked grass;
					grass.rotation = grassSeed & 0b111111U;
					grass.size = (grassSeed >> 6U) & 0b11U;
					float rotation = (float)(grass.rotation) / 64.0f * 6.283185f;
					float size = sizeBase + sizeGrowth * (float)(grass.size);
					float u = ((float)((grassSeed >>  8U) & 0b11111111U) / (float)(0b11111111U));
					float v = ((float)((grassSeed >> 16U) & 0b11111111U) / (float)(0b11111111U));
					if(u + v > 1.0f) 
					{
						u = 1.0f - u;
						v = 1.0f - v;
					} 
					glm::vec3 grassPosition = 
					vertexPosition3 + 
					dv1 * u +
					dv2 * v;

					glm::vec3 grassF = grassPosition + glm::vec3(glm::sin(rotation), 0.0f, glm::cos(rotation)) * size;
					glm::vec3 grassR = grassPosition + glm::vec3(glm::cos(rotation), 0.0f, -glm::sin(rotation)) * size;

					float shearF = glm::clamp((glm::dot(vertexPosition3 - grassF, flatNormal) / flatNormal.y) / size, -1.0f, 1.0f) * 0.5f + 0.5f;
					float shearR = glm::clamp((glm::dot(vertexPosition3 - grassR, flatNormal) / flatNormal.y) / size, -1.0f, 1.0f) * 0.5f + 0.5f;
					shearF *= (float)0b111111u;
					shearR *= (float)0b111111u;
					grass.shearF = shearF;
					grass.shearR = shearR;

					grass.X = (grassPosition.x / 32.0f) * (float)0b111111111u;
					grass.Y = (grassPosition.y / 32.0f) * (float)0b111111111u;
					grass.Z = (grassPosition.z / 32.0f) * (float)0b111111111u;
					grass.variant = grassSeed >> 31u;

					GrassVertexInfoPacked packedGrass;
					memset(&packedGrass, 0, sizeof(packedGrass));
					packedGrass.part32 |= grass.rotation;
					packedGrass.part32 |= grass.shearF << 6u;
					packedGrass.part32 |= grass.shearR << 12u;
					packedGrass.part32 |= grass.size << 18u;
					packedGrass.part32 |= grass.X << 20u;
					packedGrass.part32 |= (grass.Y & 7u) << 29u;
					packedGrass.part16 |= grass.Y >> 3u;
					packedGrass.part16 |= grass.Z << 6u;
					packedGrass.part16 |= grass.variant << 15u;
					grassData.push_back(packedGrass);
					/*
					
					//get two random random points on a triangle:
					float r1p1 = Noise::get(randomSeed + unpackedIndices[0] + index.x + grassId, index.y + grassId * 4, (index.z + 0xfaded101) * 4 + 0);
					float r2p1 = Noise::get(randomSeed + unpackedIndices[1] + index.x + grassId, index.y + grassId * 3, (index.z + 0xc0ffebea) * 4 + 1);
					float r1p2 = Noise::get(randomSeed + unpackedIndices[2] + index.x + grassId, index.y + grassId * 2, (index.z + 0x041de05c) * 4 + 2);
					float r2p2 = Noise::get(randomSeed + unpackedIndices[0]*3 + index.x + grassId, index.y + grassId * 1, (index.z + 0xca4471c1) * 4 + 3);
					if(r1p1 + r2p1 > 1.0f) 
					{
						r1p1 = 1.0f - r1p1;
						r2p1 = 1.0f - r2p1;
					} 
					if(r1p2 + r2p2 > 1.0f) 
					{
						r1p2 = 1.0f - r1p2;
						r2p2 = 1.0f - r2p2;
					} 
					glm::vec3 p1 = vertexPosition3 + dv1 * r1p1 + dv2 * r2p1;
					glm::vec3 p2 = vertexPosition3 + dv1 * r1p2 + dv2 * r2p2;
					glm::vec3 pd = p2 - p1;
					float h = glm::length(pd);
					if(h < 0.5f) 
					{
						glm::vec3 pdu = glm::normalize(pd);
						p1 -= pdu * 0.25f;
						p2 += pdu * 0.25f;
						h += 0.5f;
						pd += pdu * 0.5f;
					} 
					glm::vec3 grassNormal = glm::vec3(p1.z - p2.z, 0.0f, p2.x - p1.x);
					const float groundNormalFactor = 1.0f;
					int vdecbase = decVertices.size();
					decVertices.push_back(p1);
					decVertices.push_back(p2);
					decVertices.push_back(p1 + glm::vec3(0.0f, h, 0.0f));
					decVertices.push_back(p2 + glm::vec3(0.0f, h, 0.0f));
					decUVs.push_back(glm::vec2(0.0f, 0.0f));
					decUVs.push_back(glm::vec2(1.0f, 0.0f));
					decUVs.push_back(glm::vec2(0.0f, 1.0f));
					decUVs.push_back(glm::vec2(1.0f, 1.0f));
					const glm::vec3 gNormal0 = glm::normalize(grassNormal + flatNormal * groundNormalFactor);
					decNormals.push_back(gNormal0);
					decNormals.push_back(gNormal0);
					decNormals.push_back(gNormal0);
					decNormals.push_back(gNormal0);
					decIndices.push_back(vdecbase + 0);
					decIndices.push_back(vdecbase + 1);
					decIndices.push_back(vdecbase + 3);
					decIndices.push_back(vdecbase + 3);
					decIndices.push_back(vdecbase + 2);
					decIndices.push_back(vdecbase + 0);
					decVertices.push_back(p1);
					decVertices.push_back(p2);
					decVertices.push_back(p1 + glm::vec3(0.0f, h, 0.0f));
					decVertices.push_back(p2 + glm::vec3(0.0f, h, 0.0f));
					decUVs.push_back(glm::vec2(0.0f, 0.0f));
					decUVs.push_back(glm::vec2(1.0f, 0.0f));
					decUVs.push_back(glm::vec2(0.0f, 1.0f));
					decUVs.push_back(glm::vec2(1.0f, 1.0f));
					const glm::vec3 gNormal1 = glm::normalize(-grassNormal + flatNormal * groundNormalFactor);
					decNormals.push_back(gNormal1);
					decNormals.push_back(gNormal1);
					decNormals.push_back(gNormal1);
					decNormals.push_back(gNormal1);
					decIndices.push_back(vdecbase + 0 + 4);
					decIndices.push_back(vdecbase + 2 + 4);
					decIndices.push_back(vdecbase + 3 + 4);
					decIndices.push_back(vdecbase + 3 + 4);
					decIndices.push_back(vdecbase + 1 + 4);
					decIndices.push_back(vdecbase + 0 + 4);
					glm::vec3 p1r = (p1 + p2 + glm::vec3(pd.z, 0.0f, -pd.x)) * 0.5f;
					glm::vec3 p2r = (p1 + p2 - glm::vec3(pd.z, 0.0f, -pd.x)) * 0.5f;
					p1 = p1r;
					p2 = p2r;
					decVertices.push_back(p1);
					decVertices.push_back(p2);
					decVertices.push_back(p1 + glm::vec3(0.0f, h, 0.0f));
					decVertices.push_back(p2 + glm::vec3(0.0f, h, 0.0f));
					decUVs.push_back(glm::vec2(0.0f, 0.0f));
					decUVs.push_back(glm::vec2(1.0f, 0.0f));
					decUVs.push_back(glm::vec2(0.0f, 1.0f));
					decUVs.push_back(glm::vec2(1.0f, 1.0f));
					
					const glm::vec3 gNormal2 = glm::normalize(-glm::vec3(grassNormal.z, 0.0f, -grassNormal.x) + flatNormal * groundNormalFactor);
					decNormals.push_back(gNormal2);
					decNormals.push_back(gNormal2);
					decNormals.push_back(gNormal2);
					decNormals.push_back(gNormal2);
					decIndices.push_back(vdecbase + 0 + 8);
					decIndices.push_back(vdecbase + 1 + 8);
					decIndices.push_back(vdecbase + 3 + 8);
					decIndices.push_back(vdecbase + 3 + 8);
					decIndices.push_back(vdecbase + 2 + 8);
					decIndices.push_back(vdecbase + 0 + 8);
					decVertices.push_back(p1);
					decVertices.push_back(p2);
					decVertices.push_back(p1 + glm::vec3(0.0f, h, 0.0f));
					decVertices.push_back(p2 + glm::vec3(0.0f, h, 0.0f));
					decUVs.push_back(glm::vec2(0.0f, 0.0f));
					decUVs.push_back(glm::vec2(1.0f, 0.0f));
					decUVs.push_back(glm::vec2(0.0f, 1.0f));
					decUVs.push_back(glm::vec2(1.0f, 1.0f));
					
					const glm::vec3 gNormal3 = glm::normalize(glm::vec3(grassNormal.z, 0.0f, -grassNormal.x) + flatNormal * groundNormalFactor);
					decNormals.push_back(gNormal3);
					decNormals.push_back(gNormal3);
					decNormals.push_back(gNormal3);
					decNormals.push_back(gNormal3);
					decIndices.push_back(vdecbase + 0 + 12);
					decIndices.push_back(vdecbase + 2 + 12);
					decIndices.push_back(vdecbase + 3 + 12);
					decIndices.push_back(vdecbase + 3 + 12);
					decIndices.push_back(vdecbase + 1 + 12);
					decIndices.push_back(vdecbase + 0 + 12);
					*/
				}
			}
		}
	}
	//decorations.buildDataFromVectors(decVertices, decUVs, decNormals, decIndices);
	hasDecorations = hasDecorationsNew;
}

void MarchingCubesVoxelMesh::markWholeChunkDirty() 
{
	this->allDirty = true;
	cellUpdateQueueHead = 0;
	voxelUpdateQueueHead = 0;
	memset(cellUpdateMask.data(), 255, cellUpdateMask.size() * sizeof(unsigned long long));
	memset(voxelUpdateMask.data(), 255, voxelUpdateMask.size() * sizeof(unsigned long long));
}
void MarchingCubesVoxelMesh::markVoxelForUpdate(glm::ivec3 index) 
{
	if(index.x < 0 || index.y < 0 || index.z < 0 || index.x > dimensions.x || index.y > dimensions.y || index.z > dimensions.z) 
	{
		throw std::range_error("Index out of bounds");
	}
	if(allDirty) return;
	const int flatVoxelIndex = a3to1ind(index, dimensions.y + 1, dimensions.z + 1);
	const int flatVoxelIndexZ = flatVoxelIndex - 1;
	const int flatVoxelIndexY = flatVoxelIndex - (dimensions.z + 1);
	const int flatVoxelIndexX = flatVoxelIndex - (dimensions.z + 1) * (dimensions.y + 1);
	if(voxelUpdateQueueHead >= cellUpdateQueueMaxSize - 6 || cellUpdateQueueHead >= cellUpdateQueueMaxSize - 9) 
	{
		markWholeChunkDirty();
		return;
	}
	if(!getBitOfArray(voxelUpdateMask.data(), flatVoxelIndex)) 
	{
		setBitOfArray(voxelUpdateMask.data(), flatVoxelIndex, true);
		voxelUpdateQueue[voxelUpdateQueueHead] = flatVoxelIndex;
		voxelUpdateQueueHead++;
	}
	if(index.z > 0 && !getBitOfArray(voxelUpdateMask.data(), flatVoxelIndexZ)) 
	{
		setBitOfArray(voxelUpdateMask.data(), flatVoxelIndexZ, true);
		voxelUpdateQueue[voxelUpdateQueueHead] = flatVoxelIndexZ;
		voxelUpdateQueueHead++;
	}
	if(index.y > 0 && !getBitOfArray(voxelUpdateMask.data(), flatVoxelIndexY)) 
	{
		setBitOfArray(voxelUpdateMask.data(), flatVoxelIndexY, true);
		voxelUpdateQueue[voxelUpdateQueueHead] = flatVoxelIndexY;
		voxelUpdateQueueHead++;
	}
	if(index.x > 0 && !getBitOfArray(voxelUpdateMask.data(), flatVoxelIndexX)) 
	{
		setBitOfArray(voxelUpdateMask.data(), flatVoxelIndexX, true);
		voxelUpdateQueue[voxelUpdateQueueHead] = flatVoxelIndexX;
		voxelUpdateQueueHead++;
	}
	for(int i = 0; i <= 1; i++)
	for(int j = 0; j <= 1; j++)
	for(int k = 0; k <= 1; k++) 
	{
		glm::ivec3 cellIndex = index - glm::ivec3(i,j,k);
		if(
		cellIndex.x < 0 ||
		cellIndex.y < 0 ||
		cellIndex.z < 0 ||
		cellIndex.x >= dimensions.x ||
		cellIndex.y >= dimensions.y ||
		cellIndex.z >= dimensions.z 
		) continue;
		unsigned int cellIndex1D = a3to1ind(cellIndex, dimensions.y, dimensions.z);
		if(getUpdateMask(cellIndex)) continue;
		setUpdateMask(cellIndex, true);
		cellUpdateQueue[cellUpdateQueueHead] = cellIndex1D;
		cellUpdateQueueHead++;
	}
}

bool MarchingCubesVoxelMesh::getUpdateMask(glm::ivec3 index) 
{
	int flatIndex = a3to1ind(index, dimensions.y, dimensions.z);
	return cellUpdateMask[flatIndex / sizeof(unsigned long long) / 8] & 1ULL << (flatIndex % 64);
}
void MarchingCubesVoxelMesh::setUpdateMask(glm::ivec3 index, bool updateSet) 
{
	int flatIndex = a3to1ind(index, dimensions.y, dimensions.z);
	if(updateSet) 
		cellUpdateMask[flatIndex / sizeof(unsigned long long) / 8] |= 1ULL << (flatIndex % 64);
	else
		cellUpdateMask[flatIndex / sizeof(unsigned long long) / 8] &= ~(1ULL << (flatIndex % 64));
}

void MarchingCubesVoxelMesh::updateVoxel(const glm::ivec3& index) 
{
	static const int edgesPerAxis = (dimensions.x + 1) * (dimensions.y + 1) * dimensions.z;
	
	const Voxel v = this->data->getVoxelWrappedConst(index);
	for(int i = 0; i < 8; i++)
	{
		glm::ivec3 cellIndex = index - gridI[i];
		if(cellIndex.x < 0) continue;
		if(cellIndex.y < 0) continue;
		if(cellIndex.z < 0) continue;
		if(cellIndex.x >= dimensions.x) continue;
		if(cellIndex.y >= dimensions.y) continue;
		if(cellIndex.z >= dimensions.z) continue;
		unsigned int cellIndexFlat = a3to1ind(cellIndex, dimensions.y, dimensions.z);
		flagCubeIndex(cells[cellIndexFlat], i, v.amount <= 0);
	}
	const unsigned char xAxesList[] = {1, 0, 0};
	const unsigned char yAxesList[] = {2, 2, 1};
	for(int axis = 0; axis < 3; axis++) 
	{	
		glm::ivec3 axisVoxelIndex = index;
		axisVoxelIndex[axis] += 1;
		glm::ivec3 edgeIndex;
		edgeIndex[0] = index[axis ? 0 : 1];
		edgeIndex[1] = index[axis == 2 ? 1 : 2];
		edgeIndex[2] = index[axis];
		if(edgeIndex[2] >= dimensions[axis]) continue;
		if(edgeIndex[1] > dimensions[axis == 2 ? 1 : 2]) continue;
		if(edgeIndex[0] > dimensions[axis ? 0 : 1]) continue;
		const Voxel ve = this->data->getVoxelWrappedConst(axisVoxelIndex);
		const int edgeFlatIndex = a3to1ind(edgeIndex, dimensions[axis == 2 ? 1 : 2] + 1, dimensions[axis]) + edgesPerAxis * axis;
		unsigned char edgeOffset = EdgeIntersection(v.amount, ve.amount);
		edges[edgeFlatIndex].offset = edgeOffset;
		edges[edgeFlatIndex].materialId = v.amount > ve.amount ? v.voxelTypeId : ve.voxelTypeId;
	}
}

void MarchingCubesVoxelMesh::drawGrass() 
{
	if(!hasGrass()) return;
	glBindVertexArray(grassVAO);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, grassDataSize);
}
bool MarchingCubesVoxelMesh::hasGrass() 
{
	return grassDataSize != 0;
}