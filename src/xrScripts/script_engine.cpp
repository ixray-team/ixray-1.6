////////////////////////////////////////////////////////////////////////////
//	Module 		: script_engine.cpp
//	Created 	: 01.04.2004
//  Modified 	: 01.04.2004
//	Author		: Dmitriy Iassenev
//	Description : XRay Script Engine
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pch_script.h"
#include "script_engine.h"
#include "../xrEngine/IGame_ObjectFactory.h"
#include "script_process.h"

#include "script_callback_ex.h"

SCRIPTS_API CScriptEngine* g_pScriptEngine = nullptr;

void jit_command(lua_State*, LPCSTR);

CScriptEngine::CScriptEngine			()
{
	m_stack_level			= 0;
	m_reload_modules		= false;
	m_last_no_file_length	= 0;
	*m_last_no_file			= 0;
}

CScriptEngine::~CScriptEngine			()
{
	while (!m_script_processes.empty())
		remove_script_process(m_script_processes.begin()->first);

#ifdef DEBUG
	flush_log					();
#endif // DEBUG
}

void CScriptEngine::unload				()
{
	lua_settop				(lua(),m_stack_level);
	m_last_no_file_length	= 0;
	*m_last_no_file			= 0;
}

int CScriptEngine::lua_panic			(lua_State *L)
{
	print_output	(L,"PANIC",LUA_ERRRUN);
	return			(0);
}

void CScriptEngine::lua_error			(lua_State *L)
{
	print_output(L, "", LUA_ERRRUN);

	g_pScriptEngine->on_error	(L);

#if !XRAY_EXCEPTIONS
	Debug.fatal				(DEBUG_INFO,"LUA error: %s",lua_tostring(L,-1));
#else
	throw					lua_tostring(L,-1);
#endif
}

int CScriptEngine::lua_pcall_failed(lua_State* L)
{
	print_output(L, "", LUA_ERRRUN);
	g_pScriptEngine->on_error(L);

#if !XRAY_EXCEPTIONS
	Debug.fatal(DEBUG_INFO, "LUA error: %s", lua_isstring(L, -1) ? lua_tostring(L, -1) : "");
#endif
	if (lua_isstring(L, -1))
		lua_pop(L, 1);

	return (LUA_ERRRUN);
}

void lua_cast_failed					(lua_State *L, luabind::type_id const& info)
{
	CScriptEngine::print_output	(L,"",LUA_ERRRUN);

	Debug.fatal				(DEBUG_INFO,"LUA error: cannot cast lua value to %s",info.name());
}

void CScriptEngine::setup_callbacks		()
{
	{
#ifdef LUABIND_NO_EXCEPTIONS
		luabind::set_error_callback		(CScriptEngine::lua_error);
#endif

#ifndef MASTER_GOLD
		luabind::set_pcall_callback
		(
			[](lua_State* L) ->void
			{  
				lua_pushcfunction(L, CScriptEngine::lua_pcall_failed);
			}
		);
#endif
	}

#ifdef LUABIND_NO_EXCEPTIONS
	luabind::set_cast_failed_callback	(lua_cast_failed);
#endif

	lua_atpanic							(lua(),CScriptEngine::lua_panic);
	luabind::disable_super_deprecation();
}

#ifdef DEBUG
#	include "script_thread.h"
void CScriptEngine::lua_hook_call		(lua_State *L, lua_Debug *dbg)
{
	if (g_pScriptEngine->current_thread())
		g_pScriptEngine->current_thread()->script_hook(L,dbg);
}
#endif

int auto_load				(lua_State *L)
{
	if ((lua_gettop(L) < 2) || !lua_istable(L,1) || !lua_isstring(L,2)) {
		lua_pushnil	(L);
		return		(1);
	}

	g_pScriptEngine->process_file_if_exists(lua_tostring(L,2),false);
	lua_rawget		(L,1);
	return			(1);
}

void CScriptEngine::setup_auto_load		()
{
	luaL_newmetatable					(lua(),"XRAY_AutoLoadMetaTable");
	lua_pushstring						(lua(),"__index");
	lua_pushcfunction					(lua(), auto_load);
	lua_settable						(lua(),-3);
	lua_pushstring 						(lua(),"_G"); 
	lua_gettable 						(lua(),LUA_GLOBALSINDEX); 
	luaL_getmetatable					(lua(),"XRAY_AutoLoadMetaTable");
	lua_setmetatable					(lua(),-2);
	//. ??????????
	// lua_settop							(lua(),-0);
}

void CScriptEngine::init()
{
	CScriptStorage::reinit();

	luabind::open(lua());
	setup_callbacks();
	g_object_factory->export_classes(lua());
	setup_auto_load();

#ifdef DEBUG
	lua_sethook(lua(), lua_hook_call, LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET, 0);
#endif

	bool								save = m_reload_modules;
	m_reload_modules = true;
	process_file_if_exists("_G", false);
	m_reload_modules = save;

	register_script_classes();
	g_object_factory->register_script();

#ifdef XRGAME_EXPORTS
	load_common_scripts();
#endif
	m_stack_level = lua_gettop(lua());
}

void CScriptEngine::remove_script_process	(const EScriptProcessors &process_id)
{
	CScriptProcessStorage::iterator	I = m_script_processes.find(process_id);
	if (I != m_script_processes.end()) {
		xr_delete						((*I).second);
		m_script_processes.erase		(I);
	}
}

void CScriptEngine::load_common_scripts()
{
#ifdef DBG_DISABLE_SCRIPTS
	return;
#endif
	string_path		S;
	FS.update_path	(S,"$game_config$","script.ltx");
	CInifile		*l_tpIniFile = new CInifile(S);
	R_ASSERT		(l_tpIniFile);
	if (!l_tpIniFile->section_exist("common")) {
		xr_delete			(l_tpIniFile);
		return;
	}

	if (l_tpIniFile->line_exist("common","script")) {
		LPCSTR			caScriptString = l_tpIniFile->r_string("common","script");
		u32				n = _GetItemCount(caScriptString);
		string256		I;
		for (u32 i=0; i<n; ++i) {
			process_file(_GetItem(caScriptString,i,I));
			xr_strcat	(I,"_initialize");
			if (object("_G",I,LUA_TFUNCTION)) {
//				lua_dostring			(lua(),xr_strcat(I,"()"));
				luabind::functor<void>	f;
				R_ASSERT				(functor(I,f));
				f						();
			}
		}
	}

	xr_delete			(l_tpIniFile);
}

void CScriptEngine::process_file_if_exists	(LPCSTR file_name, bool warn_if_not_exist)
{
	u32						string_length = xr_strlen(file_name);
	if (!warn_if_not_exist && no_file_exists(file_name,string_length))
		return;

	string_path				S,S1;
	if (m_reload_modules || (*file_name && !namespace_loaded(file_name))) {
		FS.update_path		(S,"$game_scripts$", xr_strconcat(S1,file_name,".script"));
		if (!warn_if_not_exist && !FS.exist(S))
		{
#ifdef XRSE_FACTORY_EXPORTS
			print_stack			();
			Msg					("* trying to access variable %s, which doesn't exist, or to load script %s, which doesn't exist too",file_name,S1);
#endif
			add_no_file		(file_name,string_length);
			return;
		}

#ifndef MASTER_GOLD
		Msg					("* loading script %s",S1);
#endif

		m_reload_modules	= false;
		load_file_into_namespace(S,*file_name ? file_name : "_G");
	}
}

void CScriptEngine::process_file	(LPCSTR file_name)
{
	process_file_if_exists	(file_name,true);
}

void CScriptEngine::process_file	(LPCSTR file_name, bool reload_modules)
{
	m_reload_modules		= reload_modules;
	process_file_if_exists	(file_name,true);
	m_reload_modules		= false;
}

void CScriptEngine::register_script_classes		()
{
#ifdef DBG_DISABLE_SCRIPTS
	return;
#endif
	string_path					S;
	FS.update_path				(S,"$game_config$","script.ltx");
	CInifile					*l_tpIniFile = new CInifile(S);
	R_ASSERT					(l_tpIniFile);

	if (!l_tpIniFile->section_exist("common")) {
		xr_delete				(l_tpIniFile);
		return;
	}

	m_class_registrators		= READ_IF_EXISTS(l_tpIniFile,r_string,"common","class_registrators","");
	xr_delete					(l_tpIniFile);

	u32 n = _GetItemCount(*m_class_registrators);
	string256 I;

	for (u32 i=0; i<n; ++i) 
	{
		_GetItem(*m_class_registrators,i,I);
		g_object_factory->init_script(I);
	}
}

bool CScriptEngine::function_object(LPCSTR function_to_call, luabind::object &object, int type)
{
	if (!xr_strlen(function_to_call))
		return				(false);

	string256				name_space, function;

	parse_script_namespace	(function_to_call,name_space,sizeof(name_space),function,sizeof(function));
	if (xr_strcmp(name_space,"_G")) {
		LPSTR				file_name = strchr(name_space,'.');
		if (!file_name)
			process_file	(name_space);
		else {
			*file_name		= 0;
			process_file	(name_space);
			*file_name		= '.';
		}
	}

	if (!this->object(name_space,function,type))
		return				(false);

	luabind::object			lua_namespace	= this->name_space(name_space);
	object					= lua_namespace[function];
	return					(true);
}

bool CScriptEngine::no_file_exists	(LPCSTR file_name, u32 string_length)
{
	if (m_last_no_file_length != string_length)
		return				(false);

	return					(!memcmp(m_last_no_file,file_name,string_length*sizeof(char)));
}

void CScriptEngine::add_no_file		(LPCSTR file_name, u32 string_length)
{
	m_last_no_file_length	= string_length;
	CopyMemory				(m_last_no_file,file_name,(string_length+1)*sizeof(char));
}

void CScriptEngine::collect_all_garbage	()
{
	lua_gc					(lua(),LUA_GCCOLLECT,0);
	lua_gc					(lua(),LUA_GCCOLLECT,0);
}

void CScriptEngine::on_error(lua_State* state)
{
}