// exxZERO Time Stamp AddIn. Document modified at : Thursday, March 07, 2002 14:13:00 , by user : Oles , from computer : OLES
// HUDCursor.cpp: implementation of the CHUDTarget class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "hudtarget.h"
#include "hudmanager.h"
#include "../xrEngine/GameMtlLib.h"

#include "../xrEngine/Environment.h"
#include "../xrEngine/CustomHUD.h"
#include "Entity.h"
#include "level.h"
#include "game_cl_base.h"
#include "../xrEngine/IGame_Persistent.h"
#include "ai/stalker/ai_stalker.h"
#include "uifontdefines.h"

#include "InventoryOwner.h"
#include "relation_registry.h"
#include "character_info.h"

#include "../xrEngine/string_table.h"
#include "entity_alive.h"

#include "inventory_item.h"
#include "inventory.h"
#include "ui_base.h"

u32 C_ON_ENEMY		= color_xrgb(0xff,0,0);
u32 C_ON_NEUTRAL	= color_xrgb(0xff,0xff,0x80);
u32 C_ON_FRIEND		= color_xrgb(0,0xff,0);


#define C_DEFAULT	color_xrgb(0xff,0xff,0xff)
#define C_SIZE		0.025f
#define NEAR_LIM	0.5f

#define SHOW_INFO_SPEED		0.5f
#define HIDE_INFO_SPEED		10.f


IC	float	recon_mindist	()		{
	return 2.f;
}
IC	float	recon_maxdist	()		{
	return 50.f;
}
IC	float	recon_minspeed	()		{
	return 0.5f;
}
IC	float	recon_maxspeed	()		{
	return 10.f;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHUDTarget::CHUDTarget	()
{    
	fuzzyShowInfo		= 0.f;
	RQ.range			= 0.f;
	hShader->create		("hud\\cursor","ui\\cursor");

	RQ.set				(nullptr, 0.f, -1);

	Load				();
	m_bShowCrosshair	= false;
}

CHUDTarget::~CHUDTarget	()
{
}

void CHUDTarget::net_Relcase(CObject* O)
{
	if(RQ.O == O)
		RQ.O = nullptr;

	RQR.r_clear	();
}

void CHUDTarget::Load		()
{
	HUDCrosshair.Load();
}

ICF static BOOL pick_trace_callback(collide::rq_result& result, LPVOID params)
{
	collide::rq_result* RQ = (collide::rq_result*)params;
	if(result.O){	
		*RQ				= result;
		return FALSE;
	}else{
		//получить треугольник и узнать его материал
		CDB::TRI* T		= Level().ObjectSpace.GetStaticTris()+result.element;
		if (GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flPassable)) 
			return TRUE;
	}
	*RQ					= result;
	return FALSE;
}

void CHUDTarget::CursorOnFrame ()
{
	Fvector				p1,dir;

	p1					= Device.vCameraPosition;
	dir					= Device.vCameraDirection;
	
	// Render cursor
	if(Level().CurrentEntity()){
		RQ.O			= 0; 
		RQ.range		= g_pGamePersistent->Environment().CurrentEnv->far_plane*0.99f;
		RQ.element		= -1;
		
		collide::ray_defs	RD(p1, dir, RQ.range, CDB::OPT_CULL, collide::rqtBoth);
		RQR.r_clear			();
		VERIFY				(!fis_zero(RD.dir.square_magnitude()));
		if(Level().ObjectSpace.RayQuery(RQR,RD, pick_trace_callback, &RQ, nullptr, Level().CurrentEntity()))
			clamp			(RQ.range,NEAR_LIM,RQ.range);
	}

}

void CHUDTarget::ShowCrosshair(bool b)
{
	m_bShowCrosshair = b;
}

extern ENGINE_API xr_atomic_bool g_bRendering;
void CHUDTarget::Render()
{
	VERIFY		(g_bRendering);

	CObject*	O		= Level().CurrentEntity();
	if (0==O)	return;
	CEntity*	E		= smart_cast<CEntity*>(O);
	if (0==E)	return;

	Fvector p1				= Device.vCameraPosition;
	Fvector dir				= Device.vCameraDirection;
	
	// Render cursor
	u32 C				= C_DEFAULT;
	
	Fvector				p2;
	p2.mad				(p1,dir,RQ.range);
	Fvector4			pt;
	Device.mFullTransform.transform(pt, p2);
	pt.y = -pt.y;
	float				di_size = C_SIZE/powf(pt.w,.2f);

	CGameFont* F		= UI().Font().GetFont(GRAFFITI19_FONT_NAME);
	F->SetAligment		(CGameFont::alCenter);
	F->OutSetI			(0.f,0.05f);

	if (psHUD_Flags.test(HUD_CROSSHAIR_DIST)){
		F->SetColor		(C);
		if (RQ.range >= 250.0f)
			F->OutNext		("");
		else
			F->OutNext		("%4.1f",RQ.range);
	}

	if (psHUD_Flags.test(HUD_INFO)){ 
		if (RQ.O){
			CEntityAlive*	E		= smart_cast<CEntityAlive*>	(RQ.O);
			CEntityAlive*	pCurEnt = smart_cast<CEntityAlive*>	(Level().CurrentEntity());
			PIItem			l_pI	= smart_cast<PIItem>		(RQ.O);

			if (IsGameTypeSingle())
			{
				CInventoryOwner* our_inv_owner		= smart_cast<CInventoryOwner*>(pCurEnt);

				if (E && E->g_Alive() && !E->cast_base_monster())
				{
					CInventoryOwner* others_inv_owner	= smart_cast<CInventoryOwner*>(E);
					CAI_Stalker*	pStalker		= smart_cast<CAI_Stalker*>(E);

					if ((pStalker && !pStalker->IsGhost()) || !pStalker)
					{
						if(our_inv_owner && others_inv_owner){

							switch(RELATION_REGISTRY().GetRelationType(others_inv_owner, our_inv_owner))
							{
							case ALife::eRelationTypeEnemy:
								C = C_ON_ENEMY; break;
							case ALife::eRelationTypeNeutral:
								C = C_ON_NEUTRAL; break;
							case ALife::eRelationTypeFriend:
								C = C_ON_FRIEND; break;
							}

							if (fuzzyShowInfo>0.5f)
							{
								CStringTable	strtbl		;
								F->SetColor	(subst_alpha(C,u8(iFloor(255.f*(fuzzyShowInfo-0.5f)*2.f))));
								F->OutNext	("%s", *strtbl.translate(others_inv_owner->Name()) );
								F->OutNext	("%s", *strtbl.translate(others_inv_owner->CharacterInfo().Community().id()) );
							}
						}

						fuzzyShowInfo += SHOW_INFO_SPEED*Device.fTimeDelta;
					}
				}
				else 
					if (l_pI && our_inv_owner && RQ.range < 2.0f*our_inv_owner->inventory().GetTakeDist())
					{
						if (fuzzyShowInfo>0.5f){
							F->SetColor	(subst_alpha(C,u8(iFloor(255.f*(fuzzyShowInfo-0.5f)*2.f))));
							F->OutNext	("%s",l_pI->Name/*Complex*/());
						}
						fuzzyShowInfo += SHOW_INFO_SPEED*Device.fTimeDelta;
					}
			}
			else
			{
				if (E && (E->GetfHealth()>0))
				{
					if (pCurEnt && GameID() == GAME_SINGLE){	

						if (E->g_Team() != pCurEnt->g_Team())	C = C_ON_ENEMY;
						else									C = C_ON_FRIEND;

						if (RQ.range >= recon_mindist() && RQ.range <= recon_maxdist()){
							float ddist = (RQ.range - recon_mindist())/(recon_maxdist() - recon_mindist());
							float dspeed = recon_minspeed() + (recon_maxspeed() - recon_minspeed())*ddist;
							fuzzyShowInfo += Device.fTimeDelta/dspeed;
						}else{
							if (RQ.range < recon_mindist()) fuzzyShowInfo += recon_minspeed()*Device.fTimeDelta;
							else fuzzyShowInfo = 0;
						};

						if (fuzzyShowInfo>0.5f){
							clamp(fuzzyShowInfo,0.f,1.f);
							int alpha_C = iFloor(255.f*(fuzzyShowInfo-0.5f)*2.f);
							u8 alpha_b	= u8(alpha_C & 0x00ff);
							F->SetColor	(subst_alpha(C,alpha_b));
							F->OutNext	("%s",*RQ.O->cName());
						}
					}
				};
			};

		}else{
			fuzzyShowInfo -= HIDE_INFO_SPEED*Device.fTimeDelta;

		}
		clamp(fuzzyShowInfo,0.f,1.f);
	}

	//отрендерить кружочек или крестик
	if(!m_bShowCrosshair){
		// actual rendering
		UIRender->StartPrimitive	(6, IUIRender::ptTriList, UI().m_currentPointType);
		
		Fvector2		scr_size;
//.		scr_size.set	(float(::Render->getTarget()->get_width()), float(::Render->getTarget()->get_height()));
		scr_size.set	(float(Device.TargetWidth) ,float(Device.TargetHeight));
		float			size_x = scr_size.x	* di_size;
		float			size_y = scr_size.y * di_size;

		size_y			= size_x;

		float			w_2		= scr_size.x/2.0f;
		float			h_2		= scr_size.y/2.0f;

		// Convert to screen coords
		float cx		    = (pt.x+1)*w_2;
		float cy		    = (pt.y+1)*h_2;

		//	TODO: return code back to indexed rendering since we use quads
		//	Tri 1
		UIRender->PushPoint(cx - size_x, cy + size_y, 0, C, 0, 1);
		UIRender->PushPoint(cx - size_x, cy - size_y, 0, C, 0, 0);
		UIRender->PushPoint(cx + size_x, cy + size_y, 0, C, 1, 1);
		//	Tri 2
		UIRender->PushPoint(cx + size_x, cy + size_y, 0, C, 1, 1);
		UIRender->PushPoint(cx - size_x, cy - size_y, 0, C, 0, 0);
		UIRender->PushPoint(cx + size_x, cy - size_y, 0, C, 1, 0);

		// unlock VB and Render it as triangle LIST
		UIRender->SetShader(*hShader);
		UIRender->FlushPrimitive();
	}else{
		//отрендерить прицел
		HUDCrosshair.cross_color	= C;
		HUDCrosshair.OnRender		();
	}
}

