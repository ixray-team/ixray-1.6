#pragma once
#include "../../ai_entity_definitions.h"
#include "../../../../xrEngine/EffectorPP.h"
#include "../../../CameraEffector.h"
#include "../../../../xrEngine/CameraManager.h"

class CustomBloodsuckerVampirePPEffector : public CEffectorPP 
{
protected:
	using inherited  = CEffectorPP;

	SPPInfo state;			//current state
	float	m_total;		// total PP time

public:
	CustomBloodsuckerVampirePPEffector(const SPPInfo &ppi, float life_time);
	virtual ~CustomBloodsuckerVampirePPEffector() override;

	virtual	BOOL	Process					(SPPInfo& pp) override;
};
