#pragma once

#include "R_light.h"
#include "base_lighting.h"
#include "base_color.h"
#include "../../XrCDB/xrCDB.h"


#include "embree4/rtcore.h"
#pragma comment(lib, "embree4.lib")


// Vertex, Tri Buffers
namespace Embree
{

	struct VertexEmbree
	{
		float x, y, z;

		void Set(Fvector& vertex);
		Fvector Get();
	};

	struct TriEmbree
	{
		u32 point1, point2, point3;
		void SetVertexes(CDB::TRI& triangle, Fvector* verts, VertexEmbree* emb_verts, size_t& last_index);
	};


	// ������ �������� TNEAR ��� ����������� � �����
	void SetRay1(RTCRay& rayhit, Fvector& pos, Fvector& dir, float range);
	void SetRay1(RTCRayHit& rayhit, Fvector& pos, Fvector& dir, float range);


	void errorFunction(void* userPtr, enum RTCError error, const char* str);
	void IntelEmbreeSettings(RTCDevice& device, bool avx, bool sse);

}

extern float RaytraceEmbreeProcess(R_Light& L, Fvector& P, Fvector& N, float range, void* skip);
extern void LightPointEmbree(base_color_c& C, Fvector& P, Fvector& N, base_lighting& lights, u32 flags, void* skip);