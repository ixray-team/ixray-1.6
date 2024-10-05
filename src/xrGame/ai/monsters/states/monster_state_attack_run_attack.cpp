#include "StdAfx.h"

#include "../control_animation_base.h"
#include "../control_direction_base.h"

#include "monster_state_attack_run_attack.h"

#include "../monster_velocity_space.h"


void CStateMonsterAttackRunAttack::initialize()
{
	inherited::initialize();

	this->object->m_time_last_attack_success = 0;
}

void CStateMonsterAttackRunAttack::execute()
{
	this->object->set_action(ACT_RUN);
	this->object->set_state_sound(MonsterSound::eMonsterSoundAggressive);
	this->object->anim().SetSpecParams(ASP_ATTACK_RUN);
}

void CStateMonsterAttackRunAttack::finalize()
{
	inherited::finalize();
}

void CStateMonsterAttackRunAttack::critical_finalize()
{
	inherited::critical_finalize();
}

bool CStateMonsterAttackRunAttack::check_start_conditions()
{
	float dist = this->object->MeleeChecker.distance_to_enemy(this->object->EnemyMan.get_enemy());

	if (dist > this->object->db().m_run_attack_start_dist)	return false;
	if (dist < this->object->MeleeChecker.get_min_distance())		return false;

	// check angle
	if (!this->object->control().direction().is_face_target(this->object->EnemyMan.get_enemy(), deg(30))) return false;

	// try to build path
	Fvector target_position;
	target_position.mad(this->object->Position(), this->object->Direction(), this->object->db().m_run_attack_path_dist);

	//if (!this->object->control().path_builder().build_special(target_position, u32(-1), MonsterMovement::eVelocityParamsRunAttack)) return false;
	//else this->object->path().enable_path();

	return true;
}

bool CStateMonsterAttackRunAttack::check_completion()
{
	if (!this->object->control().path_builder().is_moving_on_path() || (this->object->m_time_last_attack_success != 0)) return true;
	return false;
}