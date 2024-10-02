#include "stdafx.h"

#include "../control_animation_base.h"
#include "../control_direction_base.h"

#include "burer.h"
#include "burer_state_attack_run_around.h"

#define DIST_QUANT			10.f

CStateBurerAttackRunAround::CStateBurerAttackRunAround(CBaseMonster* obj) : inherited(obj)
{
	m_pBurer = smart_cast<CBurer*>(obj);
}

void CStateBurerAttackRunAround::initialize()
{
	inherited::initialize();

	time_started = Device.dwTimeGlobal;
	dest_direction.set(0.f, 0.f, 0.f);

	// select point
	Fvector						dir_to_enemy, dir_from_enemy;
	dir_to_enemy.sub(this->object->EnemyMan.get_enemy()->Position(), this->object->Position());
	dir_to_enemy.normalize();

	dir_from_enemy.sub(this->object->Position(), this->object->EnemyMan.get_enemy()->Position());
	dir_from_enemy.normalize();

	float dist = this->object->Position().distance_to(this->object->EnemyMan.get_enemy()->Position());

	if (dist > 30.f) {							// бежать к врагу
		selected_point.mad(this->object->Position(), dir_to_enemy, DIST_QUANT);
	}
	else if ((dist < 20.f) && (dist > 4.f)) {	// убегать от врага
		selected_point.mad(this->object->Position(), dir_from_enemy, DIST_QUANT);
		dest_direction.sub(this->object->EnemyMan.get_enemy()->Position(), selected_point);
		dest_direction.normalize();
	}
	else {											// выбрать случайную позицию
		selected_point = random_position(this->object->Position(), DIST_QUANT);
		dest_direction.sub(this->object->EnemyMan.get_enemy()->Position(), selected_point);
		dest_direction.normalize();
	}

	this->object->path().prepare_builder();
}

void CStateBurerAttackRunAround::execute()
{
	if (!fis_zero(dest_direction.square_magnitude())) {
		this->object->path().set_use_dest_orient(true);
		this->object->path().set_dest_direction(dest_direction);
	}
	else this->object->path().set_use_dest_orient(false);


	this->object->set_action(ACT_RUN);
	this->object->path().set_target_point(selected_point);
	this->object->path().set_generic_parameters();
	this->object->path().set_use_covers(false);

	this->object->set_state_sound(MonsterSound::eMonsterSoundAggressive);
}


bool CStateBurerAttackRunAround::check_start_conditions()
{
	return true;
}

bool CStateBurerAttackRunAround::check_completion()
{
	if ((time_started + this->m_pBurer->m_max_runaway_time < Device.dwTimeGlobal) ||
		(this->object->control().path_builder().is_moving_on_path() && this->object->control().path_builder().is_path_end(2.f))) {

		this->object->dir().face_target(this->object->EnemyMan.get_enemy());
		return true;
	}

	return false;
}
