#pragma once


class CPHMesh;
class CODEGeom;

struct CPHGeometryBits
{
	static void init_geom(CPHMesh& g);
	static void set_ignore_static(CODEGeom& g);
};