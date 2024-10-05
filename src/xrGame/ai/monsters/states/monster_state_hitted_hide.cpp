#include "StdAfx.h"

#include "../control_animation_base.h"
#include "../control_direction_base.h"

#include "monster_state_hitted_hide.h"

#define GOOD_DISTANCE_TO_ENEMY	10.f
#define GOOD_DISTANCE_IN_COVER	15.f
#define MIN_HIDE_TIME			3.f
#define DIST_TO_PATH_END		1.5f


void CStateMonsterHittedHide::initialize()
{
	inherited::initialize();
	this->object->path().prepare_builder();
}


void CStateMonsterHittedHide::execute()
{
	this->object->set_action(ACT_RUN);
	this->object->set_state_sound(MonsterSound::eMonsterSoundPanic);
	this->object->anim().accel_activate(eAT_Aggressive);
	this->object->anim().accel_set_braking(false);
	this->object->path().set_retreat_from_point(this->object->HitMemory.get_last_hit_position());
	this->object->path().set_generic_parameters();

}


bool CStateMonsterHittedHide::check_start_conditions()
{
	if (this->object->HitMemory.is_hit() && !this->object->EnemyMan.get_enemy()) return true;
	return false;
}


bool CStateMonsterHittedHide::check_completion()
{
	float dist = this->object->Position().distance_to(this->object->HitMemory.get_last_hit_position());

	// good dist  
	if (dist < GOOD_DISTANCE_IN_COVER) return false;
	// +hide more than 3 sec
	if (this->time_state_started + MIN_HIDE_TIME > Device.dwTimeGlobal) return false;

	return true;
}

#undef GOOD_DISTANCE_IN_COVER
#undef DIST_TO_PATH_END
