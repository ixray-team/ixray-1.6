#include "StdAfx.h"

#include "../control_animation_base.h"
#include "../control_direction_base.h"

#include "monster_state_squad_rest_follow.h"

#include "../ai_monster_squad.h"
#include "../ai_monster_squad_manager.h"

#include "space_restriction.h"

#include "state_custom_action.h"
#include "state_move_to_point.h"

#define STOP_DISTANCE	2.f
#define STAY_DISTANCE	5 * STOP_DISTANCE
#define MIN_TIME_OUT	2000
#define MAX_TIME_OUT	3000


CStateMonsterSquadRestFollow::CStateMonsterSquadRestFollow(CBaseMonster* obj) : inherited(obj)
{
	this->add_state(eStateSquad_RestFollow_Idle, xr_new<CStateMonsterCustomAction >(obj));
	this->add_state(eStateSquad_RestFollow_WalkToPoint, xr_new<CStateMonsterMoveToPointEx>(obj));
}


CStateMonsterSquadRestFollow::~CStateMonsterSquadRestFollow()
{
}


void CStateMonsterSquadRestFollow::initialize()
{
	inherited::initialize();

	SSquadCommand& command = monster_squad().get_squad(this->object)->GetCommand(this->object);
	last_point = command.position;
}


void CStateMonsterSquadRestFollow::reselect_state()
{
	SSquadCommand& command = monster_squad().get_squad(this->object)->GetCommand(this->object);
	if (command.position.distance_to(this->object->Position()) < Random.randF(STOP_DISTANCE, STAY_DISTANCE)) {
		this->select_state(eStateSquad_RestFollow_Idle);
	}
	else {
		this->select_state(eStateSquad_RestFollow_WalkToPoint);
	}
}


void CStateMonsterSquadRestFollow::check_force_state()
{
}


void CStateMonsterSquadRestFollow::setup_substates()
{
	state_ptr state = this->get_state_current();

	if (this->current_substate == eStateSquad_RestFollow_Idle) {
		SStateDataAction data;
		data.action = ACT_REST;
		data.sound_type = MonsterSound::eMonsterSoundIdle;
		data.sound_delay = this->object->db().m_dwIdleSndDelay;
		data.time_out = Random.randI(MIN_TIME_OUT, MAX_TIME_OUT);

		state->fill_data_with(&data, sizeof(SStateDataAction));

		return;
	}

	if (this->current_substate == eStateSquad_RestFollow_WalkToPoint) {
		SStateDataMoveToPointEx data;

		Fvector dest_pos = monster_squad().get_squad(this->object)->GetCommand(this->object).position;
		if (!this->object->control().path_builder().restrictions().accessible(dest_pos)) {
			data.vertex = this->object->control().path_builder().restrictions().accessible_nearest(dest_pos, data.point);
		}
		else {
			data.point = dest_pos;
			data.vertex = u32(-1);
		}

		data.action.action = ACT_WALK_FWD;
		data.accelerated = true;
		data.braking = false;
		data.accel_type = eAT_Calm;
		data.completion_dist = STOP_DISTANCE;
		data.action.sound_type = MonsterSound::eMonsterSoundIdle;
		data.action.sound_delay = this->object->db().m_dwIdleSndDelay;
		data.time_to_rebuild = u32(-1);

		state->fill_data_with(&data, sizeof(SStateDataMoveToPointEx));

		return;
	}
}

#undef  STOP_DISTANCE
#undef  STAY_DISTANCE
#undef  MIN_TIME_OUT
#undef  MAX_TIME_OUT


