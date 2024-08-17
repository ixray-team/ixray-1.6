#include "Stdafx.h"
#include "script_xr_conditions.h"
#include "script_xr_conditions_functions.cpp"

// set breakpoint on these functions for 'understanding' the sense of usage and the purpose of CAnyCallable
//bool test_adiahfuirhgarughargha(int a, int b) { return false; }
//bool test2_Afjreauhgaeughauegaoegheog(int a, int b, const xr_vector<int>& c) { return false; }

CScriptXRConditionsStorage::CScriptXRConditionsStorage() : m_pLevel{}
{
	/* for test
	REGISTER_FUNCTION_TO_XR_CONDITIONS(test_adiahfuirhgarughargha);
	REGISTER_FUNCTION_TO_XR_CONDITIONS(test2_Afjreauhgaeughauegaoegheog);

	this->getRegisteredFunctionByName("test_adiahfuirhgarughargha")(1,2);
	this->getRegisteredFunctionByName("test2_Afjreauhgaeughauegaoegheog")(1, 2, xr_vector<int>{});
	*/
}

CScriptXRConditionsStorage::~CScriptXRConditionsStorage()
{
}

void CScriptXRConditionsStorage::initialize(CLevel* pLevelManager)
{
	R_ASSERT2(pLevelManager, "you have to have a valid pointer of LevelManager! early calling?");

	m_pLevel = pLevelManager;

	REGISTER_FUNCTION_TO_SCRIPT(fighting_dist_ge);
	REGISTER_FUNCTION_TO_SCRIPT(surge_started);
	REGISTER_FUNCTION_TO_SCRIPT(surge_complete);
	REGISTER_FUNCTION_TO_SCRIPT(surge_kill_all);
	REGISTER_FUNCTION_TO_SCRIPT(signal_rocket_flying);
	REGISTER_FUNCTION_TO_SCRIPT(quest_npc_enemy_actor);
	REGISTER_FUNCTION_TO_SCRIPT(animpoint_reached);
	REGISTER_FUNCTION_TO_SCRIPT(distance_to_obj_ge);
	REGISTER_FUNCTION_TO_SCRIPT(distance_to_obj_le);
	REGISTER_FUNCTION_TO_SCRIPT(in_dest_smart_cover);
	REGISTER_FUNCTION_TO_SCRIPT(active_item);
	REGISTER_FUNCTION_TO_SCRIPT(actor_nomove_nowpn);
	REGISTER_FUNCTION_TO_SCRIPT(jup_b16_is_zone_active);
	REGISTER_FUNCTION_TO_SCRIPT(check_bloodsucker_state);
	REGISTER_FUNCTION_TO_SCRIPT(dist_to_story_obj_ge);
	REGISTER_FUNCTION_TO_SCRIPT(actor_has_nimble_weapon);
	REGISTER_FUNCTION_TO_SCRIPT(actor_has_active_nimble_weapon);
}

void CScriptXRConditionsStorage::destroy()
{
}
