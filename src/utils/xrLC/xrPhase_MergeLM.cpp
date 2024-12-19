
#include "stdafx.h"
#include "build.h"

#include "xrPhase_MergeLM_Rect.h"
#include "../xrLC_Light/xrdeflector.h"
#include "../xrLC_Light/xrlc_globaldata.h"
#include "../xrLC_Light/lightmap.h"

class	pred_remove { public: IC bool	operator() (CDeflector* D) { { if (0 == D) return TRUE; }; if (D->bMerged) { D->bMerged = FALSE; return TRUE; } else return FALSE; }; };

extern BOOL _rect_place(L_rect& r, lm_layer* D);
extern void _InitSurface();


// Surface access
IC bool	sort_defl_fast(CDeflector* D1, CDeflector* D2)
{
	if (D1->layer.height < D2->layer.height)
		return true;
	else
		return false;
}


IC int	compare_defl(CDeflector* D1, CDeflector* D2)
{
	// First  - by material
	u16 M1 = D1->GetBaseMaterial();
	u16 M2 = D2->GetBaseMaterial();
	if (M1 < M2)	return	1;  // less
	if (M1 > M2)	return	0;	// more
	return				2;	// equal
}

// should define LESS(D1<D2) behaviour
// sorting - in increasing order
IC int	sort_defl_analyze(CDeflector* D1, CDeflector* D2)
{
	// first  - get material index
	u16 M1 = D1->GetBaseMaterial();
	u16 M2 = D2->GetBaseMaterial();

	// 1. material area
	u32	 A1 = pBuild->materials()[M1].internal_max_area;
	u32	 A2 = pBuild->materials()[M2].internal_max_area;
	if (A1 < A2)	return	2;	// A2 better
	if (A1 > A2)	return	1;	// A1 better

	// 2. material sector (geom - locality)
	u32	 s1 = pBuild->materials()[M1].sector;
	u32	 s2 = pBuild->materials()[M2].sector;
	if (s1 < s2)	return	2;	// s2 better
	if (s1 > s2)	return	1;	// s1 better

	// 3. just material index
	if (M1 < M2)	return	2;	// s2 better
	if (M1 > M2)	return	1;	// s1 better

	// 4. deflector area
	u32 da1 = D1->layer.Area();
	u32 da2 = D2->layer.Area();
	if (da1 < da2)return	2;	// s2 better
	if (da1 > da2)return	1;	// s1 better

	// 5. they are EQUAL
	return				0;	// equal
}

// should define LESS(D1<D2) behaviour
// sorting - in increasing order
IC bool	sort_defl_complex(CDeflector* D1, CDeflector* D2)
{
	switch (sort_defl_analyze(D1, D2))
	{
		case 1:		return true;	// 1st is better 
		case 2:		return false;	// 2nd is better
		case 0:		return false;	// none is better
		default:	return false;
	}
}


// ÍÎÂÀß ÂÅÐÑÈß

void SelectionLmapSize(vecDefl& Layer)
{
	u64 area = 0;
	int lm_1024 = 1024 * 1024;
	int lm_2048 = 2048 * 2048;
	int lm_4096 = 4096 * 4096;
	int lm_8192 = 8192 * 8192;

	for (int it = 0; it < Layer.size(); it++)
	{
		if (lm_8192 < area)
			break;

		lm_layer& L = Layer[it]->layer;
		area += L.Area();
	}

	int use_size = 8192;

	if (area < lm_1024)
	{
		use_size = 1024;
	}
	else if (area < lm_2048)
	{
		use_size = 2048;
	}
	else if (area < lm_4096)
	{
		use_size = 4096;
	}
	else if (area < lm_8192)
	{
		use_size = 8192;
	}

	clMsg("Select LM_SIZE: %d", use_size);
	setLMSIZE(use_size);
}

BOOL _rect_place_fast(L_rect& r, lm_layer* D, int _X, int _Y);
void MergeLmap(vecDefl& Layer, CLightmap* lmap, int& MERGED)
{
	// Process 	
	int _X = 0, _Y = 0;
 	u16 _Max_y = 0;

	for (int it = 0; it < Layer.size(); it++)
	{
 		if (0 == (it % 1024))
			Status("Process [%d/%d]...Merged{%d}", it, g_XSplit.size(), MERGED);

		if (_Y > getLMSIZE())
  			break;
 
		lm_layer& L = Layer[it]->layer;
		if (_Max_y < L.height + 5)
			_Max_y = L.height + 5;

		if (_X + L.width + 2 > getLMSIZE() - 16 - L.width)
		{
			_X = 0;
			_Y += _Max_y;
			_Max_y = 0;
		}

 		L_rect		rT, rS;
		rS.a.set(0, 0);
		rS.b.set(
			L.width + 2 * BORDER - 1,
			L.height + 2 * BORDER - 1);
		rS.iArea = L.Area();
		rT = rS;
 
		if (_rect_place_fast(rT, &Layer[it]->layer, _X, _Y))
		{
			BOOL		bRotated = false;
			lmap->Capture(Layer[it], rT.a.x, rT.a.y, rT.SizeX(), rT.SizeY(), bRotated);
			Layer[it]->bMerged = TRUE;
			MERGED++;
		}
	 
		_X += L.width + 5;

		Progress(float(it) / float(g_XSplit.size()));
	}
}

#include "tbb/parallel_for.h"
#include <atomic>
 
extern int CurrentArea;

void MergeOriginal(vecDefl& Layer, CLightmap* lmap, int& MERGED)
{
	u32 maxarea = getLMSIZE() * getLMSIZE() * 2;	// Max up to 8 lm selected
	u32 curarea = 0;
	u32 merge_count = 0;
	for (u32 it = 0; it < (int)Layer.size(); it++)
	{
		int		defl_area = Layer[it]->layer.Area();
		if (curarea + defl_area > maxarea)
			break;
		curarea += defl_area;
		merge_count++;
	}

	std::mutex mtx;
	int Merged = 0;
	std::atomic<u64> MergedArea = 0;	 
	std::atomic<int> Processed = 0;

	tbb::parallel_for(tbb::blocked_range<int>(0, merge_count, merge_count / 128), [&](const tbb::blocked_range<int>& range)
	{
		for (auto ID = range.begin(); ID < range.end(); ID++)
		{
			if (0 == (ID % 1024))
				Status("Process [%d/%d]... Merged[%d] Proccessed[%d], MergedArea[%d]", ID, merge_count, Merged, Processed.load(), CurrentArea);

			lm_layer& L = Layer[ID]->layer;
			L_rect		rT, rS;
			rS.a.set(0, 0);
			rS.b.set(L.width + 2 * BORDER - 1, L.height + 2 * BORDER - 1);
			rS.iArea = L.Area();
			rT = rS;

			if (_rect_place(rT, &L))
			{
				BOOL		bRotated;
				if (rT.SizeX() == rS.SizeX())
					bRotated = FALSE;
				else
					bRotated = TRUE;

				mtx.lock();
				lmap->Capture(Layer[ID], rT.a.x, rT.a.y, rT.SizeX(), rT.SizeY(), bRotated);
				Merged++;
				mtx.unlock();

				Layer[ID]->bMerged = TRUE;
				MERGED++;
			}

			Processed++;
		}		
	});
}




void CBuild::xrPhase_MergeLM()
{
 	vecDefl			Layer;

	setLMSIZE(4096);

	// **** Select all deflectors, which contain this light-layer
	Layer.clear();
	for (u32 it = 0; it < lc_global_data()->g_deflectors().size(); it++)
	{
		CDeflector* D = lc_global_data()->g_deflectors()[it];
		if (D->bMerged)		continue;
		Layer.push_back(D);
	}
  
	// Merge this layer (which left unmerged)
	while (Layer.size())
	{
		VERIFY(lc_global_data());
		string512	phase_name;
		xr_sprintf(phase_name, "Building lightmap %d...", lc_global_data()->lightmaps().size());
		Phase(phase_name);

		// Sort layer by similarity (state changes)
		// + calc material area
		Status("Selection...");
		for (u32 it = 0; it < materials().size(); it++)
			materials()[it].internal_max_area = 0;

		for (u32 it = 0; it < Layer.size(); it++)
		{
			CDeflector* D = Layer[it];
			materials()[D->GetBaseMaterial()].internal_max_area = _max(D->layer.Area(), materials()[D->GetBaseMaterial()].internal_max_area);
		}

		if (lc_global_data()->GetLMapsAlt())
		{
			std::stable_sort(Layer.begin(), Layer.end(), sort_defl_fast);
		}
		else
		{
			std::stable_sort(Layer.begin(), Layer.end(), sort_defl_complex);
		}
 
		// Select first deflectors which can fit
		Status("Selection...");

		// Startup
		_InitSurface();
		Status("Processing...");

		CLightmap* lmap = new CLightmap();
		VERIFY(lc_global_data());
		lc_global_data()->lightmaps().push_back(lmap);

		int MERGED = 0;
 		
		if (lc_global_data()->GetLMapsAlt())
		{
			MergeLmap(Layer, lmap, MERGED);
		}
		else
		{
			MergeOriginal(Layer, lmap, MERGED);
		}

		Progress(1.f);

		clMsg("MERGED: %d, TOT: %d", MERGED, Layer.size());

		// Remove merged lightmaps
		Status("Cleanup...");
		vecDeflIt last = std::remove_if(Layer.begin(), Layer.end(), pred_remove());
		Layer.erase(last, Layer.end());

		// Save
		Status("Saving...");
		VERIFY(pBuild);
		lmap->Save(pBuild->path);
	}
	VERIFY(lc_global_data());
	clMsg("%d lightmaps builded", lc_global_data()->lightmaps().size());

	// Cleanup deflectors
	Progress(1.f);
	Status("Destroying deflectors...");
	for (u32 it = 0; it < lc_global_data()->g_deflectors().size(); it++)
		xr_delete(lc_global_data()->g_deflectors()[it]);


	lc_global_data()->g_deflectors().clear();
}
 