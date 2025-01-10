#pragma once

#include "CustomZone.h"
#include "../../Include/xrRender/KinematicsAnimated.h"
#include "ZoneVisual.h"

#include "../xrScripts/script_export_space.h"

class CHairsZone : public CVisualZone {
typedef				CVisualZone		inherited;		
public:
	virtual			void		Affect				(SZoneObjectInfo* O)		;
	virtual			void		Load				(LPCSTR section);

protected:
					float		m_min_speed_to_react;
	virtual			bool		BlowoutState		();
	virtual			void		CheckForAwaking		();
					void		UpdateBlowout() override;
					void		PlayHit(CGameObject* hitted_object);

					float		CalcHitPower(float velocity, CPhysicsShellHolder* object);

					bool		ShouldIgnoreObject(CGameObject*) override;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};