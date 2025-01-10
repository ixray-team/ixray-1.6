#include "stdafx.h"
#include "controller.h"
#include "controller_state_manager.h"

#include "controller_animation.h"
#include "controller_direction.h"
#include "../control_direction_base.h"
#include "../control_movement_base.h"
#include "../control_path_builder_base.h"

#include "../controlled_entity.h"

#include "../states/monster_state_rest.h"
#include "controller_state_attack.h"
#include "../states/monster_state_attack_melee.h"
#include "../states/monster_state_attack_run.h"
#include "../states/monster_state_eat.h"
#include "../states/monster_state_panic.h"
#include "../states/monster_state_hear_int_sound.h"
#include "../states/monster_state_hear_danger_sound.h"
#include "../states/monster_state_hitted.h"
#include "../states/monster_state_attack.h"

#include "../../../entitycondition.h"

#include "../states/state_test_state.h"

#include "controller_tube.h"

CStateManagerController::CStateManagerController(CController *obj) : inherited(obj)
{
	add_state(eStateRest, new CStateMonsterRest<CController>(obj));
	add_state(eStatePanic, new CStateMonsterPanic<CController>(obj));
	add_state(eStateHearInterestingSound, new CStateMonsterHearInterestingSound<CController>(obj));
	add_state(eStateHearDangerousSound, new CStateMonsterHearDangerousSound<CController>(obj));
	add_state(eStateHitted, new CStateMonsterHitted<CController>(obj));
	// gr1ph
	add_state(eStateAttack, new CStateControllerAttack<CController>(obj));
	add_state(eStateEat, new CStateMonsterEat<CController>(obj));
	add_state(eStateCustom, new CStateControlHide<CController>(obj));
	add_state(eStateCustom_Tubeman, new CStateControllerTube<CController>(obj));
}

CStateManagerController::~CStateManagerController()
{
}

void CStateManagerController::reinit()
{
	inherited::reinit();
	object->set_mental_state(CController::eStateIdle);
}


#define FIND_ENEMY_TIME_ENEMY_HIDDEN	5000
#define FIND_ENEMY_MAX_DISTANCE			10.f

void CStateManagerController::execute()
{
	u32 state_id = u32(-1);
		
	const CEntityAlive* enemy	= object->EnemyMan.get_enemy();

	if (enemy) 
	{
		if (get_state(eStateCustom_Tubeman)->check_start_conditions())
		{
			state_id = eStateCustom_Tubeman;
		} 
		else
		{	
			state_id = eStateAttack;
		}
	} else if (object->HitMemory.is_hit()) {
		state_id = eStateHitted;
	} else if (object->hear_dangerous_sound) {
		state_id = eStateHearDangerousSound;
	} else if (object->hear_interesting_sound) {
		state_id = eStateHearInterestingSound;
	} else {
		if (can_eat())	state_id = eStateEat;
		else			state_id = eStateRest;
	}

	if (enemy) object->set_controlled_task(eTaskAttack);
	else object->set_controlled_task(eTaskFollow);

	select_state(state_id); 

	// ��������� ������� ���������
	get_state_current()->execute();

	prev_substate = current_substate;
}