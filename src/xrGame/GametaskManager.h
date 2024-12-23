#pragma once

#include "GameTaskDefs.h"
#include "object_interfaces.h"

class CGameTaskWrapper;
class CGameTask;
class CMapLocation;

class CGameTaskManager
{
	CGameTaskWrapper*		m_gametasks_wrapper;
	vGameTasks*				m_gametasks;
	enum		{
		eChanged	= (1<<0),
		eMultipleTasks = (1 << 1),
	};
	Flags8					m_flags;
	u32						m_actual_frame;
protected:
	void					UpdateActiveTask				();
public:

							CGameTaskManager				();
							~CGameTaskManager				();

    void					AllowMultipleTask				(bool allow) { m_flags.set(eMultipleTasks, allow); }

	vGameTasks&				GetGameTasks					();
	CGameTask*				HasGameTask						(const CMapLocation* ml, bool only_inprocess);
	CGameTask*				HasGameTask						(const shared_str& id, bool only_inprocess);
	CGameTask*				GiveGameTaskToActor				(CGameTask* t, u32 timeToComplete, bool bCheckExisting, u32 timer_ttl);
	void					SetTaskState					(const shared_str& id, ETaskState state);
	void					SetTaskState					(CGameTask* t, ETaskState state);

	void			UpdateTasks						();

	CGameTask*				ActiveTask						(ETaskType type = eTaskTypeStoryline);
//	void					SetActiveTask					(const shared_str& id, ETaskType type = eTaskTypeStoryline);
	void					SetActiveTask					(CGameTask* task);
	u32						ActualFrame						() const {return m_actual_frame;}
	
	CGameTask*				IterateGet						(CGameTask* t, ETaskState state, ETaskType type, bool bForward);
	u32						GetTaskIndex					(CGameTask* t, ETaskState state, ETaskType type = eTaskTypeStoryline);
	u32						GetTaskCount					(ETaskState state, ETaskType type = eTaskTypeStoryline);
	void					MapLocationRelcase				(CMapLocation* ml);

	void					ResetStorage					() {m_gametasks = NULL;};
	void					DumpTasks						();
};
