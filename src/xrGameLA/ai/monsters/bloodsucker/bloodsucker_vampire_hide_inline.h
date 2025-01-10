#pragma once
#include "../states/state_hide_from_point.h"
#include "bloodsucker_predator.h"

#define TEMPLATE_SPECIALIZATION template <\
	typename _Object\
>

#define CStateBloodsuckerVampireHideAbstract CStateBloodsuckerVampireHide<_Object>

TEMPLATE_SPECIALIZATION
CStateBloodsuckerVampireHideAbstract::CStateBloodsuckerVampireHide(_Object *obj) : inherited(obj)
{
	this->add_state	(eStateVampire_RunAway, new CStateMonsterHideFromPoint<_Object>(obj));
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireHideAbstract::reselect_state()
{
	this->select_state(eStateVampire_RunAway);
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireHideAbstract::setup_substates()
{
	state_ptr state = this->get_state_current();

	if (this->current_substate == eStateVampire_RunAway) {
		SStateHideFromPoint		data;
		data.point				= this->object->EnemyMan.get_enemy_position();
		data.accelerated		= true;
		data.braking			= false;
		data.accel_type			= eAT_Aggressive;
		data.distance			= this->object->m_vampire_runaway_distance;
		data.action.action		= ACT_RUN;
		data.action.sound_type	= MonsterSound::eMonsterSoundAggressive;
		data.action.sound_delay = this->object->db().m_dwAttackSndDelay;
		data.action.time_out	= this->object->m_vampire_runaway_time;

		state->fill_data_with(&data, sizeof(SStateHideFromPoint));

		return;
	}
}

TEMPLATE_SPECIALIZATION
bool CStateBloodsuckerVampireHideAbstract::check_completion()
{
	if ((this->current_substate == eStateVampire_RunAway) &&
		this->get_state_current()->check_completion())	return true;
	
	return false;
}

#undef TEMPLATE_SPECIALIZATION
#undef CStateBloodsuckerVampireHideAbstract