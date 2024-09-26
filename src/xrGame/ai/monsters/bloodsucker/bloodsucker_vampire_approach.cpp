#include "stdafx.h"

#include "../control_animation_base.h"
#include "../control_direction_base.h"

#include "ai_object_location.h"

#include "bloodsucker.h"
#include "bloodsucker_vampire_approach.h"

CustomBloodsuckerVampireApproach::CustomBloodsuckerVampireApproach(CustomBloodsucker* object) : inherited(object)
{
	
}

CustomBloodsuckerVampireApproach::~CustomBloodsuckerVampireApproach()
{

}

void CustomBloodsuckerVampireApproach::initialize()
{
	inherited::initialize();
	this->object->path().prepare_builder();
}

void CustomBloodsuckerVampireApproach::execute()
{
	this->object->set_action(ACT_RUN);
	this->object->anim().accel_activate(eAT_Aggressive);
	this->object->anim().accel_set_braking(false);

	u32 const target_vertex = this->object->EnemyMan.get_enemy()->ai_location().level_vertex_id();
	Fvector const target_pos = ai().level_graph().vertex_position(target_vertex);

	this->object->path().set_target_point(target_pos, target_vertex);
	this->object->path().set_rebuild_time(this->object->get_attack_rebuild_time());
	this->object->path().set_use_covers(false);
	this->object->path().set_distance_to_end(0.1f);
	this->object->set_state_sound(MonsterSound::eMonsterSoundAggressive);
}
