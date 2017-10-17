#pragma once
#include <vector>

static std::vector<float> quad_t_verts = { -1.0f, -1.0f, 0.0f,
1.0f, -1.0f, 0.0f,
1.0f, 1.0f,  0.0f,
1.0f, 1.0f,  0.0f,
-1.0f, 1.0f,  0.0f,
-1.0f, -1.0f, 0.0f };

static std::vector<float> quad_t_norms = { 0.0f, 1.0f, 0.0,
0.0f, 1.0f, 0.0,
0.0f, 1.0f, 0.0,
0.0f, 1.0f, 0.0,
0.0f, 1.0f, 0.0,
0.0f, 1.0f, 0.0 };

static std::vector<float> quad_t_texcoords = { 0, 1,
1, 1,
1, 0,
1, 0,
0, 0,
0, 1 };

static std::vector<float> quad_verts = { 0.f, 0.f, 0.f,	//top left
1.f, 0.f,  0.f,	// bottom left
1.f, 1.f,  0.f,	//bottom right
0.f, 1.f, 0.f };	// top right

static std::vector<float> quad_norms = { 0.0f, 1.0f, 0.0,
0.0f, 1.0f, 0.0,
0.0f, 1.0f, 0.0,
0.0f, 1.0f, 0.0, };

static std::vector<float> quad_texcoords = { 0, 0,
1, 0,
1, 1,
0, 1, };
