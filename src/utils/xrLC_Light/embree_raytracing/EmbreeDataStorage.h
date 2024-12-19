#pragma once
#include "stdafx.h"
#include "R_light.h"

enum ELights
{
	Hemi = 0,
	Sun = 1,
	RGB = 2
};

enum ELightType
{
	LT_Direct = 0,
	LT_Point = 1,
	LT_Secondary = 2
};

struct PackedBufferTOProcess
{
	CDB::MODEL* MDL;

	base_color_c color[8];
	Fvector position[8];
	Fvector direction[8];
	int valid[8];
	Face* skip[8];

	//u32 flags[8];
	//Face* skip[8];
	//R_Light* light;
	//float tmax[8];
	// float Dist2Light[8];
};

struct PackedBuffer
{
	CDB::MODEL* MDL = 0;

	base_color_c Color[8];
	Fvector pos[8];
	Fvector dir[8];

	u32 flags;
	Face* skip[8] = { 0 };
	R_Light* light;

	float tmax[8];
	int valid[8];
	//	float dist[8];
};

struct RayOptimizedTyped
{
	int type = 0;
	float sqD = 0;
	R_Light* Light;
	base_color_c* Color;
	Face* skip;

	Fvector pos;
	Fvector dir;

	float tmax;
	float tmin;
};

struct RayOptimizedCPU
{
	Fvector pos;
	Fvector dir;

	float tmax;
	float tmin;

	RayOptimizedCPU()
	{

	};

	RayOptimizedCPU(RayOptimizedTyped* ray)
	{
		pos = ray->pos;
		dir = ray->dir;
		tmax = ray->tmax;
		tmin = ray->tmin;
	}
};

struct Hits_XR
{
	float u, v;
	float dist;
	u32 prim;
};
