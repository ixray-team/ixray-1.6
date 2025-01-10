#pragma once
#include "../../ai_entity_definitions.h"
#include "../basemonster/base_monster.h"
#include "../../../../xrScripts/script_export_space.h"

class CChimeraBase : public CBaseMonster 
{
public:
							CChimeraBase					();
	virtual					~CChimeraBase					() override;	

	virtual void			Load						(LPCSTR section) override;
	virtual void			reinit						() override;
	virtual	void			UpdateCL					() override;

	virtual void			HitEntityInJump				(const CEntity *pEntity) override;
	virtual void			jump						(Fvector const &position, float factor) override;

private:
	virtual	char*			get_monster_class_name		() override { return (char*) "chimera"; }
	virtual EAction			CustomVelocityIndex2Action	(u32 velocity_index) override;

	using					inherited = CBaseMonster				;
	
	SVelocityParam 			m_velocity_rotate;
	SVelocityParam 			m_velocity_jump_start;

	struct attack_params
	{
		float				attack_radius;
		TTime				prepare_jump_timeout;
		TTime				attack_jump_timeout;
		TTime				stealth_timeout;
		float				force_attack_distance;
		u32					num_attack_jumps;
		u32					num_prepare_jumps;
	};

	attack_params			m_attack_params;

public:
	attack_params const&	get_attack_params			() const { return m_attack_params; }
	
	DECLARE_SCRIPT_REGISTER_FUNCTION
};