#include "stdafx.h"
#include "pseudogigant.h"
#include "pseudogigant_state_manager.h" 

#include "../control_animation_base.h"
#include "../control_direction_base.h"
#include "../control_movement_base.h"
#include "../control_path_builder_base.h"

#include "../states/monster_state_rest.h"
#include "../states/monster_state_attack.h"
#include "../states/monster_state_panic.h"
#include "../states/monster_state_eat.h"
#include "../states/monster_state_hear_int_sound.h"
#include "../states/monster_state_hear_danger_sound.h"
#include "../states/monster_state_hitted.h"
#include "../../../entitycondition.h"
#include "../states/monster_state_controlled.h"
#include "../states/monster_state_help_sound.h"

CPseudoGiantBaseStateManager::CPseudoGiantBaseStateManager(CBaseMonster* object) : inherited(object)
{
	pPseudoGiantBase = smart_cast<CPseudoGiantBase*>(object);

    add_state(eStateRest, new CStateMonsterRest(object));
    add_state(eStatePanic, new CStateMonsterPanic(object));
    add_state(eStateAttack, new CStateMonsterAttack(object));
    add_state(eStateEat, new CStateMonsterEat(object));
    add_state(eStateHearInterestingSound, new CStateMonsterHearInterestingSound(object));
    add_state(eStateHearDangerousSound, new CStateMonsterHearDangerousSound(object));
    add_state(eStateHitted, new CStateMonsterHitted(object));
    add_state(eStateControlled, new CStateMonsterControlled(object));
    add_state(eStateHearHelpSound, new CStateMonsterHearHelpSound(object));
}

CPseudoGiantBaseStateManager::~CPseudoGiantBaseStateManager()
{

}

void CPseudoGiantBaseStateManager::execute()
{
	u32 state_id = u32(-1);

	if (!pPseudoGiantBase->is_under_control()) 
	{
		const CEntityAlive* enemy	= object->EnemyMan.get_enemy();

		if (enemy) 
		{
			switch (object->EnemyMan.get_danger_type()) 
			{
			case eStrong:	
				state_id = eStatePanic; break;
			case eWeak:		
				state_id = eStateAttack; break;
			}
		} 
		else if (object->HitMemory.is_hit()) 
		{
			state_id = eStateHitted;
		}
		else if (check_state(eStateHearHelpSound)) 
		{
			state_id = eStateHearHelpSound;
		} 
		else if (object->hear_interesting_sound) 
		{
			state_id = eStateHearInterestingSound;
		} 
		else if (object->hear_dangerous_sound) 
		{
			state_id = eStateHearDangerousSound;	
		} 
		else 
		{
			if (can_eat())	
				state_id = eStateEat;
			else			
				state_id = eStateRest;
		}
	} 
	else 
		state_id = eStateControlled;

	select_state(state_id); 
	
	get_state_current()->execute();

	prev_substate = current_substate;
}
