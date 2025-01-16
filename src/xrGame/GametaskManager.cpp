#include "StdAfx.h"
#include "pch_script.h"
#include "GametaskManager.h"
#include "alife_registry_wrappers.h"
#include "../../xrUI/xrUIXmlParser.h"
#include "GameTask.h"
#include "Level.h"
#include "map_manager.h"
#include "map_location.h"
#include "Actor.h"
#include "UIGameSP.h"
#include "ui/UIPdaWnd.h"
#include "encyclopedia_article.h"
#include "ui/UIMapWnd.h"

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#pragma warning(pop)

shared_str	g_active_task_id;

struct FindTaskByID{
	shared_str	id;
	bool		b_only_inprocess;
	FindTaskByID(const shared_str& s, bool search_only_inprocess):id(s),b_only_inprocess(search_only_inprocess){}
	bool operator () (const SGameTaskKey& key)
		{
			if(b_only_inprocess)
				return (id == key.task_id && (key.getGameTask() && key.getGameTask()->GetTaskState() == eTaskStateInProgress));
			else
				return (id==key.task_id);
		}
};

bool task_prio_pred(const SGameTaskKey& k1, const SGameTaskKey& k2)
{
	return k1.getGameTask() && k2.getGameTask() && k1.getGameTask()->m_priority > k2.getGameTask()->m_priority;
}

CGameTaskManager::CGameTaskManager()
{
	m_gametasks_wrapper			= new CGameTaskWrapper();
	m_gametasks_wrapper->registry().init(0);// actor's id
	m_flags.zero				();
	m_flags.set					(eChanged, TRUE);
	m_gametasks					= nullptr;

	if( g_active_task_id.size() )
	{
		CGameTask* t = HasGameTask( g_active_task_id, true );
		if ( t )
		{
			SetActiveTask( t );
		}
	}
}

CGameTaskManager::~CGameTaskManager()
{
	delete_data					(m_gametasks_wrapper);
	g_active_task_id			= nullptr;
}

vGameTasks&	CGameTaskManager::GetGameTasks	() 
{
	if(!m_gametasks)
	{
		m_gametasks = &m_gametasks_wrapper->registry().objects();
#ifdef DEBUG
		Msg("m_gametasks size=%d",m_gametasks->size());
#endif // #ifdef DEBUG
	}

	return *m_gametasks;
}

CGameTask* CGameTaskManager::HasGameTask(const shared_str& id, bool only_inprocess)
{
	FindTaskByID key(id, only_inprocess);
	vGameTasks_it it = std::find_if(GetGameTasks().begin(),GetGameTasks().end(),key);
	if( it!=GetGameTasks().end() )
		return (*it).getGameTask();
	
	return 0;
}

CGameTask*	CGameTaskManager::GiveGameTaskToActor(CGameTask* t, u32 timeToComplete, bool bCheckExisting, u32 timer_ttl)
{
	t->CommitScriptHelperContents	();
	if(/* bCheckExisting &&*/ HasGameTask(t->m_ID, true) ) 
	{
 		Msg("! task [%s] already inprocess",t->m_ID.c_str());
		VERIFY2( 0, make_string<const char*>( "give_task : Task [%s] already inprocess!", t->m_ID.c_str()) );
		return nullptr;
	}

	m_flags.set						(eChanged, TRUE);

	GetGameTasks().push_back		(SGameTaskKey(t->m_ID) );
	GetGameTasks().back().setGameTask(t);
	t->m_ReceiveTime				= Level().GetGameTime();
	t->m_TimeToComplete				= t->m_ReceiveTime + timeToComplete * 1000; //ms
	t->m_timer_finish				= t->m_ReceiveTime + timer_ttl      * 1000; //ms

	std::stable_sort				(GetGameTasks().begin(), GetGameTasks().end(), task_prio_pred);

	t->OnArrived					();

	//CGameTask* active_task			= ActiveTask();

	//if ( (active_task == nullptr) || (active_task->m_priority < t->m_priority) )
	//{
	//	SetActiveTask( t );
	//}

	SetActiveTask( t );

	//óñòàíîâèòü ôëàæîê íåîáõîäèìîñòè ïðî÷òåíèÿ òàñêîâ â PDA
	if ( CurrentGameUI() )
		CurrentGameUI()->UpdatePda();

	t->ChangeStateCallback();

	return t;
}

void CGameTaskManager::test_groid()
{
	int a = 0;
}

void CGameTaskManager::SetTaskState(CGameTask* t, ETaskState state)
{
	PROF_EVENT("CGameTaskManager::SetTaskState");
	m_flags.set						(eChanged, TRUE);

	t->SetTaskState					(state);
	
	if ( ActiveTask() == t )
	{
		//SetActiveTask	("");
		g_active_task_id = "";
	}

	if ( CurrentGameUI() )
		CurrentGameUI()->UpdatePda();
}

void CGameTaskManager::SetTaskState(const shared_str& id, ETaskState state)
{
	CGameTask* t				= HasGameTask(id, true);
	if (nullptr==t)				{Msg("actor does not has task [%s] or it is completed", *id);	return;}
	SetTaskState				(t, state);
}

void CGameTaskManager::UpdateTasks						()
{
	if(Device.Paused())
		return;
		
	PROF_EVENT("CGameTaskManager::UpdateTasks");

	Level().MapManager().DisableAllPointers();

	if(GetGameTasks().empty())	
		return;

	
	const vGameTasks& vTasks = GetGameTasks();

	for (const SGameTaskKey& task : vTasks)
	{
		CGameTask* const pGameTask = task.getGameTask();

		if (pGameTask)
		{
			if (pGameTask->GetTaskState() != eTaskStateInProgress)
				continue;

			const ETaskState state = pGameTask->UpdateState();

			if ((state == eTaskStateFail) || (state == eTaskStateCompleted))
				SetTaskState(pGameTask, state);
		}
	}
	

	CGameTask*	pActiveTask = ActiveTask();
	if (pActiveTask)
	{
		CMapLocation* pMapLocation = pActiveTask->LinkedMapLocation();
		if ( pMapLocation && !pMapLocation->PointerEnabled() )
		{
			pMapLocation->EnablePointer();
		}
	}

	if(	m_flags.test(eChanged) )
		UpdateActiveTask();
}


void CGameTaskManager::UpdateActiveTask()
{
	std::stable_sort			(GetGameTasks().begin(), GetGameTasks().end(), task_prio_pred);

	CGameTask*	t			= ActiveTask();
	if ( !t )
	{
		CGameTask* front	= IterateGet(nullptr, eTaskStateInProgress, true);
		if ( front )
		{
			SetActiveTask	(front);
		}
	}

	m_flags.set					(eChanged, FALSE);
	m_actual_frame				= Device.dwFrame;
}

CGameTask* CGameTaskManager::ActiveTask()
{
	const shared_str&	t_id	= g_active_task_id;
	if(!t_id.size())			return nullptr;
	return						HasGameTask( t_id, true );
}
/*
void CGameTaskManager::SetActiveTask(const shared_str& id)
{
	g_active_task_id			= id;
	m_flags.set					(eChanged, TRUE);
	m_read						= true;
}*/

void CGameTaskManager::SetActiveTask(CGameTask* task)
{
	VERIFY( task );
	if ( task )
	{
		g_active_task_id		 = task->m_ID;
		m_flags.set				(eChanged, TRUE);
		task->m_read			= true;
	}
}

CUIMapWnd* GetMapWnd();

void CGameTaskManager::MapLocationRelcase(CMapLocation* ml)
{
	CUIMapWnd* mwnd = GetMapWnd();
	if(mwnd)
		mwnd->MapLocationRelcase(ml);

	CGameTask* gt = HasGameTask(ml, false);
	if(gt)
		gt->RemoveMapLocations(true);
}

CGameTask* CGameTaskManager::HasGameTask(const CMapLocation* ml, bool only_inprocess)
{
	vGameTasks_it it		= GetGameTasks().begin();
	vGameTasks_it it_e		= GetGameTasks().end();

	for(; it!=it_e; ++it)
	{
		CGameTask* gt = (*it).getGameTask();
		if(gt->LinkedMapLocation()==ml)
		{
			if(only_inprocess && gt->GetTaskState()!=eTaskStateInProgress)
				continue;

			return gt;
		}
	}
	return nullptr;
}

CGameTask* CGameTaskManager::IterateGet(CGameTask* t, ETaskState state, bool bForward)
{
	vGameTasks& v		= GetGameTasks();
	u32 cnt				= (u32)v.size();
	for(u32 i=0; i<cnt; ++i)
	{
		CGameTask* gt	= v[i].getGameTask();
		if(gt==t || nullptr==t)
		{
			bool			allow;
			if(bForward)	
			{
				if(t)		++i;
				allow		= i < cnt;
			}else
			{
				allow		= (i>0) && (--i >= 0);
			}
			if(allow)
			{
				CGameTask* found		= v[i].getGameTask();
				if ( found->GetTaskState()==state )
					return found;
				else
					return IterateGet(found, state, bForward);
			}else
				return nullptr;
		}
	}
	return nullptr;
}

u32 CGameTaskManager::GetTaskIndex( CGameTask* t, ETaskState state )
{
	if ( !t )
	{
		return 0;
	}

	vGameTasks& v	= GetGameTasks();
	u32 cnt			= (u32)v.size();
	u32 res			= 0;
	for ( u32 i = 0; i < cnt; ++i )
	{
		CGameTask* gt = v[i].getGameTask();
		if ( gt->GetTaskState() == state )
		{
			++res;
			if ( gt == t )
			{
				return res;
			}
		}
	}
	return 0;
}

u32 CGameTaskManager::GetTaskCount( ETaskState state )
{
	vGameTasks& v	= GetGameTasks();
	u32 cnt			= (u32)v.size();
	u32 res			= 0;
	for ( u32 i = 0; i < cnt; ++i )
	{
		CGameTask* gt = v[i].getGameTask();
		if ( gt->GetTaskState()==state )
		{
			++res;
		}
	}
	return res;
}

static const char* sTaskStates[]=
{
	"eTaskStateFail",
	"TaskStateInProgress",
	"TaskStateCompleted",
	"TaskStateDummy"
};

void CGameTaskManager::DumpTasks()
{
	vGameTasks_it it			= GetGameTasks().begin();
	vGameTasks_it it_e			= GetGameTasks().end();
	for(; it!=it_e; ++it)
	{
		const CGameTask* gt = (*it).getGameTask();
		Msg( " ID=[%s] state=[%s] prio=[%d] ",
			gt->m_ID.c_str(),
			sTaskStates[gt->GetTaskState()],
			gt->m_priority );
	}
}

CGameTaskManager* get_task_manager() { return Level().GameTaskManager(); }

void CGameTaskManager::script_register(lua_State* pState)
{
	if (pState)
	{
		luabind::module(pState)
			[
				// register class
				luabind::class_<CGameTaskManager>("game_task_manager")
					.def("give_task", &CGameTaskManager::GiveGameTaskToActor),

				// register globals
				luabind::def("get_game_task_manager", get_task_manager)
			];
	}
}
