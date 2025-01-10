#include "stdafx.h"
#include "bloodsucker.h"
#include "bloodsucker_state_manager.h"
#include "../../../Actor.h"
#include "../../../ActorEffector.h"
#include "../../../../Include/xrRender/KinematicsAnimated.h"
#include "../../../Level.h"
#include "../../../material_manager.h"
#include "bloodsucker_vampire_effector.h"
#include "bloodsucker_vampire_camera_effector.h"
#include "../../../detail_path_manager.h"
#include "../../../level_debug.h"
#include "../monster_velocity_space.h"
#include "../../../GamePersistent.h"
#include "../../../game_object_space.h"

#include "../control_animation_base.h"
#include "../control_movement_base.h"
#include "../control_rotation_jump.h"

#include "../../../sound_player.h"
#include "../../../../xrEngine/CameraBase.h"
#include "../../../../xrEngine/xr_level_controller.h"
#include "../../../ActorCondition.h"

#include "../../../PHDestroyable.h"
#include "../../../CharacterPhysicsSupport.h"

u32	CBloodsuckerBase::m_time_last_vampire = 0;

CBloodsuckerBase::CBloodsuckerBase()
{
	pStateManagerBase = new CBloodsuckerBaseStateManager(this);

	m_alien_control.init_external(this);
	m_drag_anim_jump = false;
	m_animated = false;
	collision_off = false;
	m_force_visibility_state = unset;
	m_runaway_invisible_time = 0;

	bone_head = nullptr;
	bone_spine = nullptr;
	collision_hit_off = false;
	invisible_particle_name = "";
	invisible_vel = {};
	j_factor = 0.f;
	j_position = {};
	m_cob = nullptr;
	m_last_critical_hit_tick = {};
	m_critical_hit_chance = 0.f;
	m_critical_hit_camera_effector_angle = 0.f;
	m_vampire_want_value = 0.f;
	m_vampire_want_speed = 0.f;
	m_vampire_wound = 0.f;
	m_vampire_gain_health = 0.f;
	m_vampire_distance = 0.f;
	m_vis_state = 0.f;
	m_full_visibility_radius = 0.f;
	m_partial_visibility_radius = 0.f;
	m_drag_anim_jump = false;
	m_visibility_state = unset;;
	m_visibility_state_last_changed_time = {};
	m_client_effector = false;
	m_hits_before_vampire = 0;
	m_predator = false;
	m_str_cel = "";
	m_sufficient_hits_before_vampire = 0;
	m_sufficient_hits_before_vampire_random = 0;
	m_time_lunge = 0;
	m_vampire_min_delay = 0;
	m_visibility_state_change_min_delay = 0;
	m_visual_predator = "";
}

CBloodsuckerBase::~CBloodsuckerBase()
{
	xr_delete	(pStateManagerBase);
}

void CBloodsuckerBase::Load(LPCSTR section)
{
	inherited::Load(section);

	if(pSettings->line_exist(section,"collision_hit_off"))
	{
		collision_hit_off = true;
	}
	else 
		collision_hit_off = false;

	if(!pSettings->line_exist(section,"is_friendly"))
		com_man().add_ability			(ControlCom::eControlRunAttack);	

	com_man().add_ability			(ControlCom::eControlRotationJump);
	com_man().add_ability			(ControlCom::eControlJump);

	invisible_vel.set				(0.1f, 0.1f);

	EnemyMemory.init_external		(this, 40000);

	anim().AddReplacedAnim(&m_bDamaged,			eAnimRun,		eAnimRunDamaged);
	anim().AddReplacedAnim(&m_bDamaged,			eAnimWalkFwd,	eAnimWalkDamaged);
	anim().AddReplacedAnim(&m_bDamaged,			eAnimStandIdle,	eAnimStandDamaged);
	anim().AddReplacedAnim(&m_bRunTurnLeft,		eAnimRun,		eAnimRunTurnLeft);
	anim().AddReplacedAnim(&m_bRunTurnRight,	eAnimRun,		eAnimRunTurnRight);


	anim().accel_load			(section);
	anim().accel_chain_add		(eAnimWalkFwd,		eAnimRun);
	anim().accel_chain_add		(eAnimWalkFwd,		eAnimRunTurnLeft);
	anim().accel_chain_add		(eAnimWalkFwd,		eAnimRunTurnRight);
	anim().accel_chain_add		(eAnimWalkDamaged,	eAnimRunDamaged);


	SVelocityParam &velocity_none		= move().get_velocity(MonsterMovement::eVelocityParameterIdle);	
	SVelocityParam &velocity_turn		= move().get_velocity(MonsterMovement::eVelocityParameterStand);
	SVelocityParam &velocity_walk		= move().get_velocity(MonsterMovement::eVelocityParameterWalkNormal);
	SVelocityParam &velocity_run		= move().get_velocity(MonsterMovement::eVelocityParameterRunNormal);
	SVelocityParam &velocity_walk_dmg	= move().get_velocity(MonsterMovement::eVelocityParameterWalkDamaged);
	SVelocityParam &velocity_run_dmg	= move().get_velocity(MonsterMovement::eVelocityParameterRunDamaged);
	SVelocityParam &velocity_steal		= move().get_velocity(MonsterMovement::eVelocityParameterSteal);

	if (pSettings->line_exist(section,"is_no_fx"))
	{
		anim().AddAnim(eAnimStandIdle,		"stand_idle_",			-1, &velocity_none,		PS_STAND);
		anim().AddAnim(eAnimStandDamaged,	"stand_damaged_",		-1, &velocity_none,		PS_STAND);
		anim().AddAnim(eAnimStandTurnLeft,	"stand_turn_ls_",		-1, &velocity_turn,		PS_STAND);
		anim().AddAnim(eAnimStandTurnRight,	"stand_turn_rs_",		-1, &velocity_turn,		PS_STAND);
		anim().AddAnim(eAnimSleep,			"lie_sleep_",			-1, &velocity_none,		PS_LIE);
		anim().AddAnim(eAnimSleepStanding,	"stand_sleep_",			-1, &velocity_none,		PS_STAND);
		anim().AddAnim(eAnimWalkFwd,		"stand_walk_fwd_",		-1, &velocity_walk,		PS_STAND);
		anim().AddAnim(eAnimWalkDamaged,	"stand_walk_fwd_dmg_",	-1, &velocity_walk_dmg,	PS_STAND);
		anim().AddAnim(eAnimRun,			"stand_run_",			-1,	&velocity_run,		PS_STAND);
		anim().AddAnim(eAnimRunDamaged,		"stand_run_dmg_",		-1,	&velocity_run_dmg,	PS_STAND);


		anim().AddAnim(eAnimRunTurnLeft,	"stand_run_turn_left_",	-1, &velocity_run,		PS_STAND);
		anim().AddAnim(eAnimRunTurnRight,	"stand_run_turn_right_",-1, &velocity_run,		PS_STAND);
		anim().AddAnim(eAnimScared,			"stand_scared_",		-1, &velocity_none,		PS_STAND);	

		anim().AddAnim(eAnimCheckCorpse,	"stand_check_corpse_",	-1,	&velocity_none,		PS_STAND);
		anim().AddAnim(eAnimEat,			"sit_eat_",				-1, &velocity_none,		PS_SIT);

		anim().AddAnim(eAnimDie,			"stand_idle_",			-1, &velocity_none,		PS_STAND);

		anim().AddAnim(eAnimAttack,			"stand_attack_",		-1, &velocity_turn,		PS_STAND);
		anim().AddAnim(eAnimAttackRun,		"stand_attack_run_",	-1, &velocity_run,		PS_STAND);

		anim().AddAnim(eAnimLookAround,		"stand_look_around_",	-1, &velocity_none,		PS_STAND);
		anim().AddAnim(eAnimSitIdle,		"sit_idle_",			-1, &velocity_none,		PS_SIT);
		anim().AddAnim(eAnimSitStandUp,		"sit_stand_up_",		-1, &velocity_none,		PS_SIT);
		anim().AddAnim(eAnimSitToSleep,		"sit_sleep_down_",		-1, &velocity_none,		PS_SIT);
		anim().AddAnim(eAnimStandSitDown,	"stand_sit_down_",		-1, &velocity_none,		PS_STAND);

		anim().AddAnim(eAnimSteal,			"stand_steal_",			-1, &velocity_steal,	PS_STAND);

		anim().AddAnim(eAnimThreaten,		"stand_threaten_",		-1, &velocity_none,		PS_STAND);
		anim().AddAnim(eAnimMiscAction_00,	"stand_to_aggressive_",	-1, &velocity_none,		PS_STAND);	
	} 
	else
	{
		anim().AddAnim(eAnimStandIdle,		"stand_idle_",			-1, &velocity_none,		PS_STAND, 	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimStandDamaged,	"stand_damaged_",		-1, &velocity_none,		PS_STAND, 	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimStandTurnLeft,	"stand_turn_ls_",		-1, &velocity_turn,		PS_STAND, 	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimStandTurnRight,	"stand_turn_rs_",		-1, &velocity_turn,		PS_STAND, 	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimSleep,			"lie_sleep_",			-1, &velocity_none,		PS_LIE,	  	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimSleepStanding,	"stand_sleep_",			-1, &velocity_none,		PS_STAND,   "fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimWalkFwd,		"stand_walk_fwd_",		-1, &velocity_walk,		PS_STAND, 	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimWalkDamaged,	"stand_walk_fwd_dmg_",	-1, &velocity_walk_dmg,	PS_STAND, 	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimRun,			"stand_run_",			-1,	&velocity_run,		PS_STAND, 	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimRunDamaged,		"stand_run_dmg_",		-1,	&velocity_run_dmg,	PS_STAND, 	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");

		
		anim().AddAnim(eAnimRunTurnLeft,	"stand_run_turn_left_",	-1, &velocity_run,		PS_STAND,	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimRunTurnRight,	"stand_run_turn_right_",-1, &velocity_run,		PS_STAND,	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimScared,			"stand_scared_",		-1, &velocity_none,		PS_STAND,	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");	

		anim().AddAnim(eAnimCheckCorpse,	"stand_check_corpse_",	-1,	&velocity_none,		PS_STAND, 	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimEat,			"sit_eat_",				-1, &velocity_none,		PS_SIT,		"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		
		anim().AddAnim(eAnimDie,			"stand_idle_",			-1, &velocity_none,		PS_STAND,	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		
		anim().AddAnim(eAnimAttack,			"stand_attack_",		-1, &velocity_turn,		PS_STAND,	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimAttackRun,		"stand_attack_run_",	-1, &velocity_run,		PS_STAND,	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");

		anim().AddAnim(eAnimLookAround,		"stand_look_around_",	-1, &velocity_none,		PS_STAND,	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimSitIdle,		"sit_idle_",			-1, &velocity_none,		PS_SIT,		"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimSitStandUp,		"sit_stand_up_",		-1, &velocity_none,		PS_SIT,		"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimSitToSleep,		"sit_sleep_down_",		-1, &velocity_none,		PS_SIT,		"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimStandSitDown,	"stand_sit_down_",		-1, &velocity_none,		PS_STAND,	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		
		anim().AddAnim(eAnimSteal,			"stand_steal_",			-1, &velocity_steal,	PS_STAND,	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		
		anim().AddAnim(eAnimThreaten,		"stand_threaten_",		-1, &velocity_none,		PS_STAND,	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");
		anim().AddAnim(eAnimMiscAction_00,	"stand_to_aggressive_",	-1, &velocity_none,		PS_STAND,	"fx_run_f", "fx_stand_b", "fx_stand_l", "fx_stand_r");	
	}

	anim().AddTransition(eAnimStandSitDown,	eAnimSleep,		eAnimSitToSleep,	false);
	anim().AddTransition(PS_STAND,			eAnimSleep,		eAnimStandSitDown,	true);
	anim().AddTransition(PS_STAND,			PS_SIT,			eAnimStandSitDown,	false);
	anim().AddTransition(PS_STAND,			PS_LIE,			eAnimStandSitDown,	false);
	anim().AddTransition(PS_SIT,			PS_STAND,		eAnimSitStandUp,	false);
	anim().AddTransition(PS_LIE,			PS_STAND,		eAnimSitStandUp,	false);

	anim().LinkAction(ACT_STAND_IDLE,	eAnimStandIdle);
	anim().LinkAction(ACT_SIT_IDLE,		eAnimSitIdle);
	anim().LinkAction(ACT_LIE_IDLE,		eAnimSitIdle);
	anim().LinkAction(ACT_WALK_FWD,		eAnimWalkFwd);
	anim().LinkAction(ACT_WALK_BKWD,	eAnimWalkBkwd);
	anim().LinkAction(ACT_RUN,			eAnimRun);
	anim().LinkAction(ACT_EAT,			eAnimEat);
	anim().LinkAction(ACT_SLEEP,		eAnimSleep);
	anim().LinkAction(ACT_REST,			eAnimSitIdle);
	anim().LinkAction(ACT_ATTACK,		eAnimAttack);
	anim().LinkAction(ACT_STEAL,		eAnimSteal);
	anim().LinkAction(ACT_LOOK_AROUND,	eAnimLookAround); 

	m_hits_before_vampire		=	0;

	#ifdef DEBUG	
		anim().accel_chain_test		();
	#endif
	
	// load other misc stuff
	invisible_vel.set				(pSettings->r_float(section,"Velocity_Invisible_Linear"),pSettings->r_float(section,"Velocity_Invisible_Angular"));
	movement().detail().add_velocity(MonsterMovement::eVelocityParameterInvisible,CDetailPathManager::STravelParams(invisible_vel.linear, invisible_vel.angular));

	LoadVampirePPEffector			(pSettings->r_string(section,"vampire_effector"));
	m_vampire_min_delay				= pSettings->r_u32(section,"Vampire_Delay");

	m_visual_predator				= pSettings->r_string(section,"Predator_Visual");

	m_vampire_want_speed			= pSettings->r_float(section,"Vampire_Want_Speed");
	m_vampire_wound					= pSettings->r_float(section,"Vampire_Wound");
	m_vampire_gain_health			= pSettings->r_float(section,"Vampire_GainHealth");
	m_vampire_distance				= pSettings->r_float(section,"Vampire_Distance");
	m_sufficient_hits_before_vampire	=	pSettings->r_u32(section,"Vampire_Sufficient_Hits");
	m_sufficient_hits_before_vampire_random	=	-1 + (rand()%3);

	invisible_particle_name			= pSettings->r_string(section,"Particle_Invisible");

	READ_IF_EXISTS(pSettings, r_float, section, "separate_factor", 0.f);

	m_visibility_state_change_min_delay	 = READ_IF_EXISTS(pSettings, r_u32, section, 
		EntityDefinitions::CBloodsuckerBase::visibilityStateChangeMinDelayString,
		EntityDefinitions::CBloodsuckerBase::defaultVisibilityStateChangeMinDelay);

	m_full_visibility_radius		=	READ_IF_EXISTS(pSettings, r_float, section, 
		EntityDefinitions::CBloodsuckerBase::fullVisibilityRadiusString,
		EntityDefinitions::CBloodsuckerBase::defaultPartialVisibilityRadius);

	m_partial_visibility_radius		=	READ_IF_EXISTS(	pSettings, r_float, section, 
		EntityDefinitions::CBloodsuckerBase::partialVisibilityRadiusString,
		EntityDefinitions::CBloodsuckerBase::defaultPartialVisibilityRadius);

	m_visibility_state						=	unset;
	m_visibility_state_last_changed_time	=	0;

	PostLoad							(section);
}

void CBloodsuckerBase::reinit()
{
	m_force_visibility_state	=	unset;

	inherited::reinit			();
	CControlledActor::reinit	();
	m_visual_default			= cNameVisual();

	if(CCustomMonster::use_simplified_visual())	return;

	Bones.Reset					();

	com_man().ta_fill_data(anim_triple_vampire, "vampire_0", "vampire_1", "vampire_2", TA_EXECUTE_LOOPED, TA_DONT_SKIP_PREPARE, 0);
	

	m_alien_control.reinit();
	
	state_invisible				= false;

	com_man().add_rotation_jump_data("run_turn_l_0","run_turn_l_1","run_turn_r_0","run_turn_r_1", PI_DIV_2);
	
	com_man().load_jump_data("boloto_jump_prepare",0, "boloto_jump_fly", "boloto_jump_end", u32(-1), MonsterMovement::eBloodsuckerVelocityParameterJumpGround,0);

	// save visual	
	m_visual_default			= cNameVisual();

	m_vampire_want_value		= 0.f;
	m_predator					= false;
	m_vis_state					= 0;

	start_invisible_predator();
}

void CBloodsuckerBase::reload(LPCSTR section)
{
	inherited::reload(section);

	sound().add(pSettings->r_string(section,"Sound_Vampire_Grasp"),				DEFAULT_SAMPLE_COUNT,	SOUND_TYPE_MONSTER_ATTACKING, MonsterSound::eHighPriority + 4,	MonsterSound::eBaseChannel,	eVampireGrasp,					"bip01_head");
	sound().add(pSettings->r_string(section,"Sound_Vampire_Sucking"),			DEFAULT_SAMPLE_COUNT,	SOUND_TYPE_MONSTER_ATTACKING, MonsterSound::eHighPriority + 3,	MonsterSound::eBaseChannel,	eVampireSucking,				"bip01_head");
	sound().add(pSettings->r_string(section,"Sound_Vampire_Hit"),				DEFAULT_SAMPLE_COUNT,	SOUND_TYPE_MONSTER_ATTACKING, MonsterSound::eHighPriority + 2,	MonsterSound::eBaseChannel,	eVampireHit,					"bip01_head");
	sound().add(pSettings->r_string(section,"Sound_Vampire_StartHunt"),			DEFAULT_SAMPLE_COUNT,	SOUND_TYPE_MONSTER_ATTACKING, MonsterSound::eHighPriority + 5,	MonsterSound::eBaseChannel,	eVampireStartHunt,				"bip01_head");
	sound().add(pSettings->r_string(section,"Sound_Invisibility_Change_State"),	DEFAULT_SAMPLE_COUNT,	SOUND_TYPE_MONSTER_ATTACKING, MonsterSound::eNormalPriority,	MonsterSound::eChannelIndependent << 1,	eChangeVisibility,	"bip01_head");
	sound().add(pSettings->r_string(section,"Sound_Growl"),						DEFAULT_SAMPLE_COUNT,	SOUND_TYPE_MONSTER_ATTACKING, MonsterSound::eHighPriority + 6,	MonsterSound::eBaseChannel,	eGrowl,							"bip01_head");
	sound().add(pSettings->r_string(section,"Sound_Alien"),						DEFAULT_SAMPLE_COUNT,	SOUND_TYPE_MONSTER_ATTACKING, MonsterSound::eCriticalPriority,	u32(MonsterSound::eCaptureAllChannels),	eAlien,				"bip01_head");
}

void CBloodsuckerBase::LoadVampirePPEffector(LPCSTR section)
{
	pp_vampire_effector.duality.h			= pSettings->r_float(section,"duality_h");
	pp_vampire_effector.duality.v			= pSettings->r_float(section,"duality_v");
	pp_vampire_effector.gray				= pSettings->r_float(section,"gray");
	pp_vampire_effector.blur				= pSettings->r_float(section,"blur");
	pp_vampire_effector.noise.intensity		= pSettings->r_float(section,"noise_intensity");
	pp_vampire_effector.noise.grain			= pSettings->r_float(section,"noise_grain");
	pp_vampire_effector.noise.fps			= pSettings->r_float(section,"noise_fps");

	VERIFY(!fis_zero(pp_vampire_effector.noise.fps));

	if (sscanf(pSettings->r_string(section, "color_base"), "%f,%f,%f", 
		&pp_vampire_effector.color_base.r, 
		&pp_vampire_effector.color_base.g, 
		&pp_vampire_effector.color_base.b) != 3)
	{
		Msg("! Failed to parse color_base in section %s", section);
	}

	if (sscanf(pSettings->r_string(section, "color_gray"), "%f,%f,%f", 
		&pp_vampire_effector.color_gray.r, 
		&pp_vampire_effector.color_gray.g, 
		&pp_vampire_effector.color_gray.b) != 3)
	{
		Msg("! Failed to parse color_gray in section %s", section);
	}

	if (sscanf(pSettings->r_string(section, "color_add"), "%f,%f,%f", 
		&pp_vampire_effector.color_add.r, 
		&pp_vampire_effector.color_add.g, 
		&pp_vampire_effector.color_add.b) != 3)
	{
		Msg("! Failed to parse color_add in section %s", section);
	}
}

void  CBloodsuckerBase::BoneCallback(CBoneInstance *B)
{
	CBloodsuckerBase*	this_class = static_cast<CBloodsuckerBase*> (B->callback_param());

	this_class->Bones.Update(B, Device.dwTimeGlobal);
}

void CBloodsuckerBase::vfAssignBones()
{
	bone_spine =	&smart_cast<IKinematics*>(Visual())->LL_GetBoneInstance(smart_cast<IKinematics*>(Visual())->LL_BoneID("bip01_spine"));
	bone_head =		&smart_cast<IKinematics*>(Visual())->LL_GetBoneInstance(smart_cast<IKinematics*>(Visual())->LL_BoneID("bip01_head"));

	if(!PPhysicsShell())
	{
		bone_spine->set_callback(bctCustom,BoneCallback,this);
		bone_head->set_callback(bctCustom,BoneCallback,this);
	}

	Bones.Reset();
	Bones.AddBone(bone_spine, AXIS_X);	Bones.AddBone(bone_spine, AXIS_Y);
	Bones.AddBone(bone_head, AXIS_X);	Bones.AddBone(bone_head, AXIS_Y);
}

void CBloodsuckerBase::ActivateVampireEffector()
{
	Actor()->Cameras().AddCamEffector(new CustomBloodsuckerVampireCameraEffector(6.0f, get_head_position(this), get_head_position(Actor())));
	Actor()->Cameras().AddPPEffector(new CustomBloodsuckerVampirePPEffector(pp_vampire_effector, 6.0f));
}

bool CBloodsuckerBase::WantVampire()
{
	return							!!fsimilar(m_vampire_want_value,1.f);
}

void CBloodsuckerBase::SatisfyVampire ()
{
	m_vampire_want_value		=	0.f;

	float health				=	conditions().GetHealth();
	health						+=	m_vampire_gain_health;

	health						=	_min(health, conditions().GetMaxHealth());
	conditions().SetHealth			(health);
}

void CBloodsuckerBase::CheckSpecParams(u32 spec_params)
{
	if ((spec_params & ASP_CHECK_CORPSE) == ASP_CHECK_CORPSE) {
		com_man().seq_run(anim().get_motion_id(eAnimCheckCorpse));
	}

	if ((spec_params & ASP_THREATEN) == ASP_THREATEN) {
		anim().SetCurAnim(eAnimThreaten);
		return;
	}

	if ((spec_params & ASP_STAND_SCARED) == ASP_STAND_SCARED) {
		anim().SetCurAnim(eAnimLookAround);
		return;
	}

}

BOOL CBloodsuckerBase::net_Spawn (CSE_Abstract* DC)
{
	if (!inherited::net_Spawn(DC))
		return(FALSE);

	vfAssignBones();

	return(TRUE);
}

float   CBloodsuckerBase::get_full_visibility_radius ()
{
	return override_if_debug(EntityDefinitions::CBloodsuckerBase::fullVisibilityRadiusString, m_full_visibility_radius);
}

float   CBloodsuckerBase::get_partial_visibility_radius ()
{
	return override_if_debug(EntityDefinitions::CBloodsuckerBase::partialVisibilityRadiusString, m_partial_visibility_radius);
}

TTime   CBloodsuckerBase::get_visibility_state_change_min_delay ()
{
	return override_if_debug(EntityDefinitions::CBloodsuckerBase::visibilityStateChangeMinDelayString, m_visibility_state_change_min_delay);
}

CBloodsuckerBase::visibility_t   CBloodsuckerBase::get_visibility_state () const
{
	return m_force_visibility_state != unset ? m_force_visibility_state : m_visibility_state;
}

void   CBloodsuckerBase::set_visibility_state (visibility_t new_state)
{
	if ( m_force_visibility_state != unset )
	{
		new_state			=	m_force_visibility_state;
	}

	if ( new_state == unset )
	{
		return;
	}

	if ( m_visibility_state == new_state )
	{
		return;
	}

	if ( Device.dwTimeGlobal <	m_visibility_state_last_changed_time + 
								get_visibility_state_change_min_delay() )
	{
		return;
	}

	m_visibility_state_last_changed_time	=	Device.dwTimeGlobal;

	m_visibility_state						=	new_state;

	if ( m_visibility_state == full_visibility )
	{
		stop_invisible_predator		();
	}
	else if ( m_visibility_state == partial_visibility )
	{
		start_invisible_predator	();
	}
	else
	{
		sound().play				(eChangeVisibility);
	}
}

void   CBloodsuckerBase::force_visibility_state (int state)
{
	m_force_visibility_state	=	(visibility_t)state;
	set_visibility_state			((visibility_t)state);
}

void   CBloodsuckerBase::update_invisibility ()
{
	if (CCustomMonster::use_simplified_visual())	return;

	if ( !g_Alive() )
	{
		set_visibility_state				(full_visibility);
	}
	else if ( Device.dwTimeGlobal < m_runaway_invisible_time + EntityDefinitions::CBloodsuckerBase::defaultRunawayInvisibleTime/6 )
	{
		set_visibility_state				(partial_visibility);
	}
	else if ( Device.dwTimeGlobal < m_runaway_invisible_time + EntityDefinitions::CBloodsuckerBase::defaultRunawayInvisibleTime)
	{
		set_visibility_state				(no_visibility);
	}
	else if ( CEntityAlive const * const enemy = EnemyMan.get_enemy() )
	{
		float const dist2enemy			=	enemy->Position().distance_to(Position());

		if ( dist2enemy <= get_full_visibility_radius() )
		{
			set_visibility_state			(full_visibility);
		}
		else if ( dist2enemy <= get_partial_visibility_radius() )
		{
			set_visibility_state			(partial_visibility);
		}
		else
		{
			set_visibility_state			(no_visibility);
		}
	}
	else
	{
		set_visibility_state				(full_visibility);
	}
}

u8 CBloodsuckerBase::GetCustomSyncFlag() const
{
	Flags8 flag{};
	flag.zero();

	switch (get_visibility_state())
	{
	case unset:
		flag.set(f_unset, true);
		break;
	case no_visibility:
		flag.set(f_no_visibility, true);
		break;
	case partial_visibility:
		flag.set(f_partial_visibility, true);
		break;
	case full_visibility:
		flag.set(f_full_visibility, true);
		break;
	default:
		R_ASSERT(0);
		break;
	}

	return flag.flags;
}

void CBloodsuckerBase::ProcessCustomSyncFlag_CL(u8 flags)
{
	Flags8 flag{};
	flag.flags = flags;

	visibility_t new_state;

	if (flag.test(f_no_visibility))
	{
		new_state = no_visibility;
	}
	else if (flag.test(f_partial_visibility))
	{
		new_state = partial_visibility;
	}
	else if (flag.test(f_full_visibility))
	{
		new_state = full_visibility;
	}
	else
	{
		new_state = unset;
	}

	if (get_visibility_state() == new_state)
		return;

	m_visibility_state_last_changed_time = Device.dwTimeGlobal;

	m_visibility_state = new_state;

	if (m_visibility_state == full_visibility)
	{
		stop_invisible_predator();
	}
	else if (m_visibility_state == partial_visibility)
	{
		start_invisible_predator();
	}
	else
	{
		sound().play(eChangeVisibility);
	}
}

void CBloodsuckerBase::UpdateCL()
{
	update_invisibility				();
	inherited::UpdateCL				();
	CControlledActor::frame_update	();
	character_physics_support()->movement()->CollisionEnable(!is_collision_off());

	if (g_Alive())
	{
		// update vampire need
		m_vampire_want_value += m_vampire_want_speed * client_update_fdelta();
		clamp(m_vampire_want_value,0.f,1.f);
	}
}

void CBloodsuckerBase::shedule_Update(u32 dt)
{
	inherited::shedule_Update(dt);
	
	if (!g_Alive())	
	{
		setVisible(TRUE);
		if ( state_invisible )
		{
			stop_invisible_predator();
		}
	}

	if (m_alien_control.active())	sound().play(eAlien);

	if (IsGameTypeSingle() || OnServer())
		return;

	if (!CControlledActor::is_turning() && !m_client_effector) {
		IKinematics* pK = smart_cast<IKinematics*>(Visual());
		Fmatrix bone_transform{};
		bone_transform = pK->LL_GetTransform(pK->LL_BoneID("bip01_head"));

		Fmatrix global_transform{};
		global_transform.mul_43(XFORM(), bone_transform);

		CControlledActor::look_point(global_transform.c);
		sound().play(eVampireSucking);
		ActivateVampireEffector();
		m_client_effector = true;
	}

}

void CBloodsuckerBase::Die(CObject* who)
{
	inherited::Die(who);
	stop_invisible_predator();
}

void CBloodsuckerBase::post_fsm_update()
{
	inherited::post_fsm_update();
}

bool CBloodsuckerBase::check_start_conditions(ControlCom::EControlType type)
{
	if ( type == ControlCom::eControlJump )
	{
		return false;
	}

	if ( !inherited::check_start_conditions(type) )
	{
		return false;
	}

	if ( type == ControlCom::eControlRunAttack )
	{
		return !state_invisible;
	}

	return true;
}

void CBloodsuckerBase::set_alien_control(bool val)
{
	val ? m_alien_control.activate() : m_alien_control.deactivate();
}

void CBloodsuckerBase::set_vis()
{
	m_vis_state = 1;
	predator_stop();
}

void CBloodsuckerBase::set_invis()
{
	m_vis_state = -1;
	predator_start();
}

void CBloodsuckerBase::set_collision_off(bool b_collision)
{
	collision_off = b_collision;
}

bool CBloodsuckerBase::is_collision_off()
{
	return collision_off;
}

void CBloodsuckerBase::jump(const Fvector &position, float factor)
{
	com_man().script_jump	(position, factor);
	sound().play			(MonsterSound::eMonsterSoundAggressive);
}

void CBloodsuckerBase::set_drag_jump(CEntityAlive* e, LPCSTR s, const Fvector &position, float factor)
{
	j_position = position;
	j_factor = factor;
	m_cob = e;
	m_str_cel = s;
	m_drag_anim_jump = true;
	m_animated = true;
}

bool CBloodsuckerBase::is_drag_anim_jump()
{
	return m_drag_anim_jump;
}

bool CBloodsuckerBase::is_animated()
{
	return m_animated;
}

void CBloodsuckerBase::start_drag()
{
	if(m_animated){
		com_man().script_capture(ControlCom::eControlAnimation);
		smart_cast<IKinematicsAnimated*>(Visual())->PlayCycle("boloto_attack_link_bone",TRUE,animation_end_jump,this);
		m_animated = false;
	}
}

void CBloodsuckerBase::animation_end_jump(CBlend* B)
{
	((CBloodsuckerBase*)B->CallbackParam)->set_invis();
	((CBloodsuckerBase*)B->CallbackParam)->jump(((CBloodsuckerBase*)B->CallbackParam)->j_position, ((CBloodsuckerBase*)B->CallbackParam)->j_factor);
}

void CBloodsuckerBase::predator_start()
{
	if( m_vis_state!=0 )
	{
		if( m_vis_state==1 )
		{
			return;
		}
		m_predator = false;
	}
	if (m_predator)					return;

	cNameVisual_set(m_visual_predator);
	CDamageManager::reload(*cNameSect(),"damage",pSettings);


	if (IsGameTypeSingle() || OnServer())
		control().animation().restart();
	else
	{
		MotionID mid{};
		mid.idx = u_last_motion_idx;
		mid.slot = u_last_motion_slot;
		if (mid.valid() && u_last_motion_idx != u16(-1) && u_last_motion_slot != u16(-1)) {
			u_last_motion_idx = NULL;
			u_last_motion_slot = NULL;
			u8 loop = u_last_motion_no_loop;

			// 			ApplyAnimation(mid.idx, mid.slot, u_last_motion_no_loop); <---------- OMP
			MotionID motion;
			IKinematicsAnimated* ik_anim_obj = smart_cast<IKinematicsAnimated*>(Visual());
			if (u_last_motion_idx != mid.idx || u_last_motion_slot != mid.slot)
			{
				u_last_motion_idx = mid.idx;
				u_last_motion_slot = mid.slot;
				u_last_motion_no_loop = loop;
				motion.idx = mid.idx;
				motion.slot = mid.slot;
				if (motion.valid())
				{
					u16 bone_or_part = ik_anim_obj->LL_GetMotionDef(motion)->bone_or_part;
					if (bone_or_part == u16(-1)) bone_or_part = ik_anim_obj->LL_PartID("default");

					CStepManager::on_animation_start(motion, ik_anim_obj->LL_PlayCycle(bone_or_part, motion, TRUE,
						ik_anim_obj->LL_GetMotionDef(motion)->Accrue(), ik_anim_obj->LL_GetMotionDef(motion)->Falloff(),
						ik_anim_obj->LL_GetMotionDef(motion)->Speed(), loop, 0, 0, 0));
				}
			}
		}
	}
	
	CParticlesPlayer::StartParticles(invisible_particle_name,Fvector().set(0.0f,0.1f,0.0f),ID());		
	sound().play					(eChangeVisibility);

	m_predator						= true;
}

void CBloodsuckerBase::predator_stop()
{
	if( m_vis_state != 0 )
	{
		if( m_vis_state == -1)
		{
			return;
		}

		m_predator = true;
	}

	if ( !m_predator )
	{
		return;
	}
	
	cNameVisual_set(*m_visual_default);
	character_physics_support()->in_ChangeVisual();

	CDamageManager::reload(*cNameSect(),"damage",pSettings);

	if (IsGameTypeSingle() || OnServer())
		control().animation().restart();
	else
	{
		MotionID mid{};
		mid.idx = u_last_motion_idx;
		mid.slot = u_last_motion_slot;
		if (mid.valid() && u_last_motion_idx != u16(-1) && u_last_motion_slot != u16(-1)) {
			u_last_motion_idx = NULL;
			u_last_motion_slot = NULL;
			u8 loop = u_last_motion_no_loop;

			// 			ApplyAnimation(mid.idx, mid.slot, u_last_motion_no_loop); <---------- OMP
			MotionID motion;
			IKinematicsAnimated* ik_anim_obj = smart_cast<IKinematicsAnimated*>(Visual());
			if (u_last_motion_idx != mid.idx || u_last_motion_slot != mid.slot)
			{
				u_last_motion_idx = mid.idx;
				u_last_motion_slot = mid.slot;
				u_last_motion_no_loop = loop;
				motion.idx = mid.idx;
				motion.slot = mid.slot;
				if (motion.valid())
				{
					u16 bone_or_part = ik_anim_obj->LL_GetMotionDef(motion)->bone_or_part;
					if (bone_or_part == u16(-1)) bone_or_part = ik_anim_obj->LL_PartID("default");

					CStepManager::on_animation_start(motion, ik_anim_obj->LL_PlayCycle(bone_or_part, motion, TRUE,
						ik_anim_obj->LL_GetMotionDef(motion)->Accrue(), ik_anim_obj->LL_GetMotionDef(motion)->Falloff(),
						ik_anim_obj->LL_GetMotionDef(motion)->Speed(), loop, 0, 0, 0));
				}
			}
		}
	}

	CParticlesPlayer::StartParticles(invisible_particle_name,Fvector().set(0.0f,0.1f,0.0f),ID());		
	sound().play					(eChangeVisibility);
	m_predator						= false;
}

void CBloodsuckerBase::predator_freeze()
{
	control().animation().freeze	();
}

void CBloodsuckerBase::predator_unfreeze()
{
	control().animation().unfreeze();
}

void CBloodsuckerBase::move_actor_cam (float angle)
{
	if ( Actor()->cam_Active() ) 
	{
		Actor()->cam_Active()->Move(Random.randI(2) ? kRIGHT : kLEFT, angle);
		Actor()->cam_Active()->Move(Random.randI(2) ? kUP	 : kDOWN, angle);
	}
}

void CBloodsuckerBase::HitEntity(const CEntity *pEntity, float fDamage, float impulse, Fvector &dir, ALife::EHitType hit_type, bool draw_hit_marks)
{
	bool is_critical = rand()/(float)RAND_MAX <= m_critical_hit_chance;

	if ( is_critical )
	{
		impulse *= 10.f;
	}

	inherited::HitEntity(pEntity, fDamage, impulse, dir, hit_type, draw_hit_marks);
}

bool CBloodsuckerBase::in_solid_state ()
{
	return true;
}

void CBloodsuckerBase::Hit(SHit* pHDS)
{
	if ( !collision_hit_off )
	{
		inherited::Hit(pHDS);
	}
}

void CBloodsuckerBase::start_invisible_predator()
{
	state_invisible	= true;
	predator_start();
}

void CBloodsuckerBase::stop_invisible_predator()
{
	state_invisible	= false;
	predator_stop();
}

void CBloodsuckerBase::manual_activate()
{
	state_invisible = true;
	setVisible		(FALSE);
}

void CBloodsuckerBase::manual_deactivate()
{
	state_invisible = false;
	setVisible		(TRUE);
}

void   CBloodsuckerBase::renderable_Render ()
{
	if ( m_visibility_state != no_visibility )
	{
		inherited::renderable_Render();
	}
}

bool   CBloodsuckerBase::done_enough_hits_before_vampire ()
{
	return (int)m_hits_before_vampire >= (int)m_sufficient_hits_before_vampire + m_sufficient_hits_before_vampire_random;
}

void   CBloodsuckerBase::on_attack_on_run_hit ()
{
	++m_hits_before_vampire;	
}

void   CBloodsuckerBase::force_stand_sleep_animation (u32 index)
{
	anim().set_override_animation(eAnimSleepStanding, index);
}

void   CBloodsuckerBase::release_stand_sleep_animation ()
{
	anim().clear_override_animation();

}

void CBloodsuckerBase::sendToStartVampire(CActor* pA)
{
	NET_Packet	tmp_packet{};
	CGameObject::u_EventGen(tmp_packet, GE_BLOODSUCKER_VAMPIRE_START, ID());
	//tmp_packet.w_u16(pA->ID());
	CSE_Abstract* e_who = Level().Server->ID_to_entity(pA->ID());
	xrClientData* xrCData = e_who->owner;
	Level().Server->SendTo(xrCData->ID, tmp_packet, net_flags(TRUE, TRUE));
}

void CBloodsuckerBase::sendToStopVampire()
{
	NET_Packet	tmp_packet{};
	CGameObject::u_EventGen(tmp_packet, GE_BLOODSUCKER_VAMPIRE_STOP, ID());
	//tmp_packet.w_u16(CControlledActor::m_actor->ID());
	CSE_Abstract* e_who = Level().Server->ID_to_entity(CControlledActor::m_actor->ID());
	xrClientData* xrCData = e_who->owner;
	Level().Server->SendTo(xrCData->ID, tmp_packet, net_flags(TRUE, TRUE));
}

#include "../../../HudManager.h"
void CBloodsuckerBase::OnEvent(NET_Packet& P, u16 type)
{
	inherited::OnEvent(P, type);

	switch (type)
	{
	case GE_BLOODSUCKER_VAMPIRE_START: {
		if (Actor())
		{
			CControlledActor::install(Actor());
			IKinematics* pK = smart_cast<IKinematics*>(Visual());
			Fmatrix bone_transform{};
			bone_transform = pK->LL_GetTransform(pK->LL_BoneID("bip01_head"));

			Fmatrix global_transform{};
			global_transform.mul_43(XFORM(), bone_transform);

			CControlledActor::look_point(global_transform.c);

			HUD().SetRenderable(false);
			NET_Packet			P{};
			Actor()->u_EventGen(P, GEG_PLAYER_WEAPON_HIDE_STATE, Actor()->ID());
			P.w_u16(INV_STATE_BLOCK_ALL);
			P.w_u8(u8(true));
			Actor()->u_EventSend(P);

			sound().play(eVampireGrasp);
			m_client_effector = false;
			Actor()->set_inventory_disabled(true);
		}
		break;
	}
	case GE_BLOODSUCKER_VAMPIRE_STOP: {
		if (Actor())
		{
			if (CControlledActor::is_controlling())
				CControlledActor::release();

			HUD().SetRenderable(true);
			NET_Packet			P{};
			Actor()->u_EventGen(P, GEG_PLAYER_WEAPON_HIDE_STATE, Actor()->ID());
			P.w_u16(INV_STATE_BLOCK_ALL);
			P.w_u8(u8(false));
			Actor()->u_EventSend(P);

			sound().play(eVampireHit);
			Actor()->set_inventory_disabled(false);
		}
		break;
	}
									break;
	}
}

#ifdef DEBUG
CBaseMonster::SDebugInfo CBloodsuckerBase::show_debug_info()
{
	CBaseMonster::SDebugInfo info = inherited::show_debug_info();
	if (!info.active) return CBaseMonster::SDebugInfo();

	string128 text;
	xr_sprintf(text, "Vampire Want Value = [%f] Speed = [%f]", m_vampire_want_value, m_vampire_want_speed);
	DBG().text(this).add_item(text, info.x, info.y+=info.delta_y, info.color);
	DBG().text(this).add_item("---------------------------------------", info.x, info.y+=info.delta_y, info.delimiter_color);

	return CBaseMonster::SDebugInfo();
}

// Lain: added
void   CBloodsuckerBase::add_debug_info (debug::text_tree& root_s)
{
}
#endif // DEBUG

