#include "stdafx.h"
#include "EmbreeRayTrace.h"

#include "../../xrcdb/xrcdb.h"

#include "xrLC_GlobalData.h"
#include "xrface.h"
#include "xrdeflector.h"
#include "light_point.h"
#include "R_light.h"

//Intel Code Start

#include "EmbreeDataStorage.h"
#include <atomic>

#include "embree4/rtcore.h"
#pragma comment(lib, "embree4.lib")
RTCScene IntelScene = 0;
RTCScene IntelSceneTransp = 0;

int LastGeometryID = RTC_INVALID_GEOMETRY_ID;

RTCGeometry IntelGeometryNormal = 0;
RTCGeometry IntelGeometryTransp = 0;

struct VertexEmbree
{
	float x, y, z;

	void Set(Fvector& vertex)
	{
		x = vertex.x;
		y = vertex.y;
		z = vertex.z;
	}

	Fvector Get()
	{
		Fvector vertex;
		x = vertex.x;
		y = vertex.y;
		z = vertex.z;
		return vertex;
	}
};

struct TriEmbree
{
	uint32_t point1, point2, point3;
	void SetVertexes(Fvector* verts, VertexEmbree* emb_verts, size_t& last_index)
	{
		point1 = last_index;
		point2 = last_index + 1;
		point3 = last_index + 2;

		emb_verts[last_index].Set(verts[point1]);
		emb_verts[last_index + 1].Set(verts[point2]);
		emb_verts[last_index + 2].Set(verts[point3]);

		last_index += 3;
	}
};

/** NORMAL GEOM **/
VertexEmbree* verticesNormal = 0;
TriEmbree* trianglesNormal = 0;
u32 SizeTriangleNormal = 0;
size_t SizeVertexNormal = 0;
xr_vector<void*> TriNormal_Dummys;

RTCDevice device;

// ВАЖНЫЙ ПАРАМЕТР TNEAR Для пересечения с водой
void SetRay1(RayOptimizedCPU* ray, RTCRay& rayhit)
{
	rayhit.dir_x = ray->dir.x;
	rayhit.dir_y = ray->dir.y;
	rayhit.dir_z = ray->dir.z;

	rayhit.org_x = ray->pos.x;
	rayhit.org_y = ray->pos.y;
	rayhit.org_z = ray->pos.z;

	rayhit.tnear = ray->tmin;
	rayhit.tfar = ray->tmax;

	rayhit.mask = (unsigned int)(-1);
	rayhit.flags = 0;
}

void SetRay1(RayOptimizedCPU* ray, RTCRayHit& rayhit)
{
	rayhit.ray.dir_x = ray->dir.x;
	rayhit.ray.dir_y = ray->dir.y;
	rayhit.ray.dir_z = ray->dir.z;

	rayhit.ray.org_x = ray->pos.x;
	rayhit.ray.org_y = ray->pos.y;
	rayhit.ray.org_z = ray->pos.z;

	rayhit.ray.tnear = ray->tmin;
	rayhit.ray.tfar = ray->tmax;

	rayhit.ray.mask = (unsigned int)(-1);
	rayhit.ray.flags = 0;

	rayhit.hit.Ng_x = 0;
	rayhit.hit.Ng_y = 0;
	rayhit.hit.Ng_z = 0;

	rayhit.hit.u = 0;
	rayhit.hit.v = 0;

	rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
	rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
	rayhit.hit.primID = RTC_INVALID_GEOMETRY_ID;
}


// Сделать потом переключалку

struct RayQueryContext
{
	RTCRayQueryContext context;
	Fvector B;

	Face* skip = 0;
	R_Light* Light = 0;
	float energy = 1.0f;
	u32 LastPremitive = 0;
};

void FilterRaytrace(const struct RTCFilterFunctionNArguments* args)
{
	RayQueryContext* ctxt = (RayQueryContext*)args->context;
	RTCHit* hit = (RTCHit*)args->hit;
	RTCRay* ray = (RTCRay*)args->ray;

	if (hit->primID == RTC_INVALID_GEOMETRY_ID || ctxt->LastPremitive == hit->primID)
		return;

	ctxt->LastPremitive = hit->primID;

	base_Face* F = (base_Face*)(TriNormal_Dummys[hit->primID]);

	args->valid[0] = 0;

	if (!F || F == ctxt->skip)
 		return;
 

	if (!F->flags.bOpaque)
	{
		// Перемещаем начало луча немного дальше пересечения
		b_material& M = inlc_global_data()->materials()[F->dwMaterial];
		b_texture& T = inlc_global_data()->textures()[M.surfidx];

		if (T.pSurface == nullptr)
		{
			ray->tfar = -std::numeric_limits<float>::infinity();
			ctxt->energy = 0;
			args->valid[0] = -1;	// Стоп для поиска дальнейшого 
			return;
		}

		// barycentric coords
		// note: W,U,V order
		ctxt->B.set(1.0f - hit->u - hit->v, hit->u, hit->v);

		//// calc UV
		Fvector2* cuv = F->getTC0();
		Fvector2	uv;
		uv.x = cuv[0].x * ctxt->B.x + cuv[1].x * ctxt->B.y + cuv[2].x * ctxt->B.z;
		uv.y = cuv[0].y * ctxt->B.x + cuv[1].y * ctxt->B.y + cuv[2].y * ctxt->B.z;
		int U = iFloor(uv.x * float(T.dwWidth) + .5f);
		int V = iFloor(uv.y * float(T.dwHeight) + .5f);
		U %= T.dwWidth;		if (U < 0) U += T.dwWidth;
		V %= T.dwHeight;	if (V < 0) V += T.dwHeight;

		u32 pixel = T.pSurface[V * T.dwWidth + U];
		u32 pixel_a = color_get_A(pixel);
		float opac = 1.f - _sqr(float(pixel_a) / 255.f);

		// Дополнение Контекста
		ctxt->energy *= opac;

		if (ctxt->energy < 0.1f)
		{
			ray->tfar = -std::numeric_limits<float>::infinity();
			ctxt->energy = 0;
			args->valid[0] = -1;	// Стоп для поиска дальнейшого 
		}
	}


	if (F->flags.bOpaque)
	{
		// При нахождении любого хита сразу все попали в непрозрачный Face.
		ray->tfar = -std::numeric_limits<float>::infinity();
		ctxt->energy = 0;
		args->valid[0] = -1;	// Стоп для поиска дальнейшого 
		return;
	}

}

float RaytraceEmbreeProcess(R_Light& L, Fvector& P, Fvector& N, float range, Face* skip)
{
	RayQueryContext data_hits;
	data_hits.Light = &L;
	data_hits.skip = skip;
	data_hits.energy = 1.0f;
	data_hits.LastPremitive = RTC_INVALID_GEOMETRY_ID;


	RayOptimizedCPU ray;
	ray.pos = P;
	ray.dir = N;
	ray.tmax = range;

	// Сраная вода
	ray.tmin = 0.001f;


	// Start Raytrce buffer
	RTCRayQueryContext context;
	rtcInitRayQueryContext(&context);
	data_hits.context = context;

	RTCIntersectArguments args;
	rtcInitIntersectArguments(&args);
	args.context = &data_hits.context;
	args.flags = RTCRayQueryFlags::RTC_RAY_QUERY_FLAG_INCOHERENT;

	RTCRayHit ray_occluded;
	SetRay1(&ray, ray_occluded);
	rtcIntersect1(IntelScene, &ray_occluded, &args);

	return data_hits.energy;
}

void errorFunction(void* userPtr, enum RTCError error, const char* str)
{
	clMsg("error %d: %s", error, str);
	DebugBreak();
}

// OFF PACKED PROCESSING
void GetEmbreeDeviceProperty(LPCSTR msg, RTCDevice& device, RTCDeviceProperty prop)
{
	clMsg("EmbreeDevProp: %s : %llu", msg, rtcGetDeviceProperty(device, prop));
}

void IntelEmbreeSettings(bool avx, bool sse)
{
 	GetEmbreeDeviceProperty("RTC_DEVICE_PROPERTY_RAY_MASK_SUPPORTED", device, RTC_DEVICE_PROPERTY_RAY_MASK_SUPPORTED);
	GetEmbreeDeviceProperty("RTC_DEVICE_PROPERTY_BACKFACE_CULLING_ENABLED", device, RTC_DEVICE_PROPERTY_BACKFACE_CULLING_ENABLED);
	GetEmbreeDeviceProperty("RTC_DEVICE_PROPERTY_NATIVE_RAY4_SUPPORTED", device, RTC_DEVICE_PROPERTY_NATIVE_RAY4_SUPPORTED);

	GetEmbreeDeviceProperty("RTC_DEVICE_PROPERTY_NATIVE_RAY8_SUPPORTED", device, RTC_DEVICE_PROPERTY_NATIVE_RAY8_SUPPORTED);
	GetEmbreeDeviceProperty("RTC_DEVICE_PROPERTY_NATIVE_RAY16_SUPPORTED", device, RTC_DEVICE_PROPERTY_NATIVE_RAY16_SUPPORTED);
	GetEmbreeDeviceProperty("RTC_DEVICE_PROPERTY_IGNORE_INVALID_RAYS_ENABLED", device, RTC_DEVICE_PROPERTY_IGNORE_INVALID_RAYS_ENABLED);

	GetEmbreeDeviceProperty("RTC_DEVICE_PROPERTY_TASKING_SYSTEM", device, RTC_DEVICE_PROPERTY_TASKING_SYSTEM);
}


void InitializeGeometryAttach_CDB(RTCScene& scene, CDB::CollectorPacked& packed_cb)
{
	Fvector* CDB_verts = packed_cb.getV();
	CDB::TRI* CDB_tris = packed_cb.getT();

	IntelGeometryNormal = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
	rtcSetGeometryBuildQuality(IntelGeometryNormal, RTCBuildQuality::RTC_BUILD_QUALITY_REFIT);

	rtcSetGeometryOccludedFilterFunction(IntelGeometryNormal, &FilterRaytrace);
	rtcSetGeometryIntersectFilterFunction(IntelGeometryNormal, &FilterRaytrace);

	int v_cnt = packed_cb.getVS();
	int t_cnt = packed_cb.getTS();
	verticesNormal = (VertexEmbree*)rtcSetNewGeometryBuffer(IntelGeometryNormal, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(VertexEmbree), v_cnt);
	trianglesNormal = (TriEmbree*)rtcSetNewGeometryBuffer(IntelGeometryNormal, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(TriEmbree), t_cnt);

	SizeTriangleNormal = t_cnt;
	SizeVertexNormal = v_cnt;
	size_t VertexIndexer = 0;

	// FIX
	TriNormal_Dummys.clear();
	TriNormal_Dummys.reserve(t_cnt);

	for (int i = 0; i < t_cnt; i++)
	{
		trianglesNormal[i].SetVertexes(CDB_verts, verticesNormal, VertexIndexer);
		TriNormal_Dummys[i] = (base_Face*) convert_nax(CDB_tris[i].dummy);
	}

	rtcCommitGeometry(IntelGeometryNormal);
	LastGeometryID = rtcAttachGeometry(scene, IntelGeometryNormal);

	rtcSetSceneFlags(scene, RTCSceneFlags::RTC_SCENE_FLAG_ROBUST | RTCSceneFlags::RTC_SCENE_FLAG_COMPACT);


	clMsg("[Intel Embree] Attached Geometry: IntelGeometry(Normal) By ID: %d", LastGeometryID);
}

void IntelEmbereLOAD(CDB::CollectorPacked& packed_cb)
{
	if (IntelScene != nullptr && LastGeometryID != RTC_INVALID_GEOMETRY_ID)
	{
		rtcDetachGeometry(IntelScene, LastGeometryID);

		if (IntelGeometryNormal != nullptr)
			rtcReleaseGeometry(IntelGeometryNormal);

		verticesNormal = 0;
		trianglesNormal = 0;
		SizeTriangleNormal = 0;
		SizeVertexNormal = 0;
		TriNormal_Dummys.clear();
		LastGeometryID = RTC_INVALID_GEOMETRY_ID;


		InitializeGeometryAttach_CDB(IntelScene, packed_cb);
		rtcCommitScene(IntelScene);
	}
	else
	{
		bool avx_test = CPU::ID.hasFeature(CPUFeature::AVX);
		bool sse = CPU::ID.hasFeature(CPUFeature::SSE);

		const char* config = "";
		if (avx_test)
			config = "threads=16,isa=avx2";
		else if (sse)
			config = "threads=16,isa=sse4.2";
		else 
			config = "threads=16,isa=sse2";

		device = rtcNewDevice(config);
		rtcSetDeviceErrorFunction(device, errorFunction, NULL);
		
		string128 phase;
		sprintf(phase, "Intilized Intel Embree %s - %s", RTC_VERSION_STRING, avx_test ? "avx" : sse ? "sse" : "default");
		Status(phase);
		IntelEmbreeSettings(avx_test, sse);

		// Создание сцены и добавление геометрии
		// Scene
		IntelScene = rtcNewScene(device);
		InitializeGeometryAttach_CDB(IntelScene, packed_cb);
		rtcCommitScene(IntelScene);
	}
}

void IntelEmbereUNLOAD()
{
	if (LastGeometryID != RTC_INVALID_GEOMETRY_ID)
	{
		rtcDetachGeometry(IntelScene, LastGeometryID);
		rtcReleaseGeometry(IntelGeometryNormal);
	}

	rtcReleaseScene(IntelScene);
	rtcReleaseDevice(device);
}
