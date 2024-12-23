#include "stdafx.h"
#include "UITaskWnd.h"
#include "UIMapWnd.h"
#include "object_broker.h"
#include "../../xrUI/UIXmlInit.h"
#include "../../xrUI/Widgets/UIStatic.h"
#include "../../xrUI/Widgets/UI3tButton.h"
#include "../../xrUI/Widgets/UIFrameLineWnd.h"
#include "UISecondTaskWnd.h"
#include "UIMapLegend.h"
#include "../../xrUI/UIHelper.h"
#include "../../xrUI/Widgets/UIHint.h"

#include "../GameTask.h"
#include "../map_location.h"
#include "../map_location_defs.h"
#include "../map_manager.h"
#include "UIInventoryUtilities.h"
#include "../Level.h"
#include "../GametaskManager.h"
#include "../Actor.h"
#include "../../xrUI/Widgets/UICheckButton.h"

CUITaskWnd::CUITaskWnd()
	: m_background(nullptr), m_background2(nullptr),
	m_center_background(nullptr), m_right_bottom_background(nullptr),
	m_task_split(nullptr), m_pMapWnd(nullptr),
	m_pStoryLineTaskItem(nullptr), m_pSecondaryTaskItem(nullptr),
	m_BtnTaskListWnd(nullptr), m_second_task_index(nullptr),
	m_devider(nullptr), m_actual_frame(0),
	m_btn_focus(nullptr), m_btn_focus2(nullptr),
	m_cbTreasures(nullptr), m_cbQuestNpcs(nullptr),
	m_cbSecondaryTasks(nullptr), m_cbPrimaryObjects(nullptr),
	m_bTreasuresEnabled(false), m_bQuestNpcsEnabled(false),
	m_bSecondaryTasksEnabled(false), m_bPrimaryObjectsEnabled(false),
	m_task_wnd(nullptr), m_task_wnd_show(false),
	m_map_legend_wnd(nullptr), hint_wnd(nullptr)
{
}
CUITaskWnd::~CUITaskWnd()
{
	delete_data						(m_pMapWnd);
}

void CUITaskWnd::Init()
{
	CUIXml							xml;
	xml.Load						(CONFIG_PATH, UI_PATH, PDA_TASK_XML);
	VERIFY							(hint_wnd);

	CUIXmlInit::InitWindow			(xml, "main_wnd", 0, this);

	m_background					= UIHelper::CreateFrameWindow( xml, "background", this, false);

	m_background2					= UIHelper::CreateFrameLine(xml, "background", this, false);

	if (xml.NavigateToNode("task_split"))
		m_task_split = UIHelper::CreateFrameLine(xml, "task_split", this);

	if (xml.NavigateToNode("filter_treasures"))
	{
		m_cbTreasures = UIHelper::CreateCheck(xml, "filter_treasures", this);
		m_cbTreasures->SetCheck(true);
		AddCallback(m_cbTreasures, BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUITaskWnd::OnShowTreasures));
	}
	m_bTreasuresEnabled = true;

	if (xml.NavigateToNode("filter_primary_objects"))
	{
		m_cbPrimaryObjects = UIHelper::CreateCheck(xml, "filter_primary_objects", this);
		m_cbPrimaryObjects->SetCheck(true);
		AddCallback(m_cbPrimaryObjects, BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUITaskWnd::OnShowPrimaryObjects));
	}
	m_bPrimaryObjectsEnabled		= true;

	if (xml.NavigateToNode("filter_secondary_tasks"))
	{
		m_cbSecondaryTasks = UIHelper::CreateCheck(xml, "filter_secondary_tasks", this);
		m_cbSecondaryTasks->SetCheck(true);
		AddCallback(m_cbSecondaryTasks, BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUITaskWnd::OnShowSecondaryTasks));
	}
	m_bSecondaryTasksEnabled		= true;

	if (xml.NavigateToNode("filter_quest_npcs"))
	{
		m_cbQuestNpcs = UIHelper::CreateCheck(xml, "filter_quest_npcs", this);
		m_cbQuestNpcs->SetCheck(true);
		AddCallback(m_cbQuestNpcs, BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUITaskWnd::OnShowQuestNpcs));
	}
	m_bQuestNpcsEnabled				= true;
	
	m_pMapWnd						= new CUIMapWnd(); 
	m_pMapWnd->SetAutoDelete		(false);
	m_pMapWnd->hint_wnd				= hint_wnd;
	m_pMapWnd->Init					(PDA_TASK_XML,"map_wnd");
	AttachChild						(m_pMapWnd);

	m_center_background				= UIHelper::CreateStatic( xml, "center_background", this );
	if (xml.NavigateToNode("line_devider"))
	{
		m_devider = UIHelper::CreateStatic(xml, "line_devider", this);
	}

	m_pStoryLineTaskItem			= new CUITaskItem();
	m_pStoryLineTaskItem->Init		(xml,"storyline_task_item");
	AttachChild						(m_pStoryLineTaskItem);
	m_pStoryLineTaskItem->SetAutoDelete(true);
	AddCallback						(m_pStoryLineTaskItem, WINDOW_LBUTTON_DB_CLICK,   CUIWndCallback::void_function(this,&CUITaskWnd::OnTask1DbClicked));

	if (xml.NavigateToNode("secondary_task_item"))
    {
        Level().GameTaskManager().AllowMultipleTask(true);
        m_pSecondaryTaskItem = new CUITaskItem();
        m_pSecondaryTaskItem->Init(xml, "secondary_task_item");
        AttachChild(m_pSecondaryTaskItem);
        m_pSecondaryTaskItem->SetAutoDelete(true);
        AddCallback(m_pSecondaryTaskItem, WINDOW_LBUTTON_DB_CLICK, CUIWndCallback::void_function(this, &CUITaskWnd::OnTask2DbClicked));
    }

	m_btn_focus		= UIHelper::Create3tButton( xml, "btn_task_focus", this );
	Register		(m_btn_focus);
	AddCallback		(m_btn_focus,  BUTTON_DOWN, CUIWndCallback::void_function(this,&CUITaskWnd::OnTask1DbClicked));

    if (xml.NavigateToNode("btn_task_focus2"))
    {
		m_btn_focus2 = UIHelper::Create3tButton(xml, "btn_task_focus2", this);
        Register(m_btn_focus2);
        AddCallback(m_btn_focus2, BUTTON_DOWN, CUIWndCallback::void_function(this, &CUITaskWnd::OnTask2DbClicked));
    }
	m_BtnTaskListWnd		= UIHelper::Create3tButton( xml, "btn_second_task", this );
	AddCallback				(m_BtnTaskListWnd, BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUITaskWnd::OnShowTaskListWnd));

	if (xml.NavigateToNode("second_task_index"))
		m_second_task_index = UIHelper::CreateStatic(xml, "second_task_index", this);

	m_task_wnd					= new UITaskListWnd(); 
	m_task_wnd->SetAutoDelete	(true);
	m_task_wnd->hint_wnd		= hint_wnd;
	m_task_wnd->init_from_xml	(xml, "second_task_wnd");
	m_pMapWnd->AttachChild		(m_task_wnd);
	m_task_wnd->SetMessageTarget(this);
	m_task_wnd->Show			(false);
	m_task_wnd_show				= false;

	m_map_legend_wnd					= new UIMapLegend(); 
	m_map_legend_wnd->SetAutoDelete		(true);
	m_map_legend_wnd->init_from_xml		(xml, "map_legend_wnd");
	m_pMapWnd->AttachChild				(m_map_legend_wnd);
	m_map_legend_wnd->SetMessageTarget	(this);
	m_map_legend_wnd->Show				(false);
}

void CUITaskWnd::Update()
{
	if(Level().GameTaskManager().ActualFrame() != m_actual_frame)
	{
		ReloadTaskInfo();
	}

	if ( m_pStoryLineTaskItem->show_hint && m_pStoryLineTaskItem->OwnerTask() )
	{
		m_pMapWnd->ShowHintTask( m_pStoryLineTaskItem->OwnerTask(), m_pStoryLineTaskItem );
	}
	else if (m_pSecondaryTaskItem && m_pSecondaryTaskItem->show_hint && m_pSecondaryTaskItem->OwnerTask())
	{
		m_pStoryLineTaskItem->show_hint = false;
		m_pMapWnd->ShowHintTask(m_pSecondaryTaskItem->OwnerTask(), m_pSecondaryTaskItem);
	}
	else
	{
		m_pMapWnd->HideCurHint();
	}
	inherited::Update				();
}

void CUITaskWnd::Draw()
{
	inherited::Draw					();
}

void CUITaskWnd::DrawHint()
{
	m_pMapWnd->DrawHint();
}


void CUITaskWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	if ( msg == PDA_TASK_SET_TARGET_MAP && pData )
	{
		CGameTask* task = static_cast<CGameTask*>( pData );
		TaskSetTargetMap( task );
		return;
	}
	if ( msg == PDA_TASK_SHOW_MAP_SPOT && pData && m_bSecondaryTasksEnabled)
	{
		CGameTask* task = static_cast<CGameTask*>( pData );
		TaskShowMapSpot( task, true );
		return;
	}
	if ( msg == PDA_TASK_HIDE_MAP_SPOT && pData )
	{
		CGameTask* task = static_cast<CGameTask*>( pData );
		TaskShowMapSpot( task, false );
		return;
	}
	
	if ( msg == PDA_TASK_SHOW_HINT && pData )
	{
		CGameTask* task = static_cast<CGameTask*>( pData );
		m_pMapWnd->ShowHintTask( task, pWnd );
		return;
	}
	if ( msg == PDA_TASK_HIDE_HINT )
	{
		m_pMapWnd->HideCurHint();
		return;
	}

	inherited::SendMessage(  pWnd, msg, pData );
	CUIWndCallback::OnEvent( pWnd, msg, pData );
}

void CUITaskWnd::ReloadTaskInfo()
{
    CGameTask* storyTask = Level().GameTaskManager().ActiveTask(eTaskTypeStoryline);
    m_pStoryLineTaskItem->InitTask(storyTask);

    CGameTask* additionalTask = nullptr;
    if (m_pSecondaryTaskItem)
    {
        additionalTask = Level().GameTaskManager().ActiveTask(eTaskTypeAdditional);
        m_pSecondaryTaskItem->InitTask(additionalTask);
    }

    if (!storyTask || (storyTask->m_map_object_id == u16(-1) || storyTask->m_map_location.size() == 0))
		m_btn_focus->Show(false);
	else
		m_btn_focus->Show(true);

	if (m_btn_focus2)
	{
		if (!additionalTask || (additionalTask->m_map_object_id == u16(-1) || additionalTask->m_map_location.size() == 0))
			m_btn_focus2->Show(false);
		else
			m_btn_focus2->Show(true);
	}

	Locations map_locs			= Level().MapManager().Locations();
	Locations_it b				= map_locs.begin(), 
				 e				= map_locs.end();
	for(;b!=e;b++)
	{
		shared_str spot = b->spot_type;
		if(spot=="treasure")
			m_bTreasuresEnabled?b->location->EnableSpot():b->location->DisableSpot();
		else if(spot=="primary_object")
			m_bPrimaryObjectsEnabled?b->location->EnableSpot():b->location->DisableSpot();
		else if(spot=="secondary_task_location" || spot=="secondary_task_location_complex_timer")
			(/*b->location->SpotEnabled() && */m_bSecondaryTasksEnabled)?b->location->EnableSpot():b->location->DisableSpot();
		else if(spot=="ui_pda2_trader_location" || spot=="ui_pda2_mechanic_location" ||
		   spot=="ui_pda2_scout_location" || spot=="ui_pda2_quest_npc_location" || 
		   spot=="ui_pda2_medic_location" || spot=="ui_pda2_actor_box_location" ||
		   spot=="ui_pda2_actor_sleep_location")
			m_bQuestNpcsEnabled?b->location->EnableSpot():b->location->DisableSpot();
	}

	if (storyTask || additionalTask)
	{
		m_actual_frame = Level().GameTaskManager().ActualFrame();
		if (m_task_wnd->IsShown())
			m_task_wnd->UpdateList();
	}

	if (!m_second_task_index)
		return;

	if (storyTask && !additionalTask)
	{
		const auto task_count = Level().GameTaskManager().GetTaskCount(eTaskStateInProgress, eTaskTypeStoryline);
		if (task_count)
		{
			const auto task_index = Level().GameTaskManager().GetTaskIndex(storyTask, eTaskStateInProgress, eTaskTypeStoryline);
			string32 buf;
			xr_sprintf(buf, sizeof(buf), "%d / %d", task_index, task_count);

			m_second_task_index->SetVisible(true);
			m_second_task_index->TextItemControl()->SetText(buf);
		}
		else
		{
			m_second_task_index->SetVisible(false);
			m_second_task_index->TextItemControl()->SetText("");
		}
	}

	if (additionalTask)
	{
		const auto task2_count = Level().GameTaskManager().GetTaskCount(eTaskStateInProgress, eTaskTypeAdditional);

		if (task2_count)
		{
			const auto task2_index = Level().GameTaskManager().GetTaskIndex(additionalTask, eTaskStateInProgress, eTaskTypeAdditional);
			string32 buf;
			xr_sprintf(buf, sizeof(buf), "%d / %d", task2_index, task2_count);

			m_second_task_index->SetVisible(true);
			m_second_task_index->TextItemControl()->SetText(buf);
		}
		else
		{
			m_second_task_index->SetVisible(false);
			m_second_task_index->TextItemControl()->SetText("");
		}
	}
}
void CUITaskWnd::Show(bool status)
{
	inherited::Show			(status);
	m_pMapWnd->Show			(status);
	m_pMapWnd->HideCurHint	();
	m_map_legend_wnd->Show	(false);
	if ( status )
	{
		ReloadTaskInfo();
		m_task_wnd->Show( m_task_wnd_show );
	}
	else
	{
//		m_task_wnd_show = false;
		m_task_wnd->Show(false);
	}
}

void CUITaskWnd::Reset()
{
	inherited::Reset	();
}

void CUITaskWnd::OnNextTaskClicked()
{
}

void CUITaskWnd::OnPrevTaskClicked()
{
}

void CUITaskWnd::OnShowTaskListWnd( CUIWindow* w, void* d )
{
	m_task_wnd_show = !m_task_wnd_show;
	m_task_wnd->Show( !m_task_wnd->IsShown() );
}

void CUITaskWnd::Show_TaskListWnd(bool status)
{
	m_task_wnd->Show( status );
	m_task_wnd_show = status;
}

void CUITaskWnd::TaskSetTargetMap( CGameTask* task )
{
	if (!task || !m_bSecondaryTasksEnabled)
	{
		return;
	}
	
	TaskShowMapSpot( task, true );
	CMapLocation* ml = task->LinkedMapLocation();
	if ( ml && ml->SpotEnabled() )
	{
		ml->CalcPosition();
		m_pMapWnd->SetTargetMap( ml->GetLevelName(), ml->GetPosition(), true );
	}
}

void CUITaskWnd::TaskShowMapSpot( CGameTask* task, bool show )
{
	if (!task || !m_bSecondaryTasksEnabled)
	{
		return;
	}

	CMapLocation* ml = task->LinkedMapLocation();
	if ( ml )
	{
		if ( show )
		{
			ml->EnableSpot();
			ml->CalcPosition();
			m_pMapWnd->SetTargetMap( ml->GetLevelName(), ml->GetPosition(), true );
		}
		else
		{
			ml->DisableSpot();
		}
	}
}

void CUITaskWnd::OnTask1DbClicked(CUIWindow*, void*)
{
    CGameTask* task = Level().GameTaskManager().ActiveTask(eTaskTypeStoryline);
    TaskSetTargetMap(task);
}

void CUITaskWnd::OnTask2DbClicked(CUIWindow*, void*)
{
    CGameTask* task = Level().GameTaskManager().ActiveTask(eTaskTypeAdditional);
    TaskSetTargetMap(task);
}

void CUITaskWnd::ShowMapLegend( bool status )
{
	m_map_legend_wnd->Show( status );
}

void CUITaskWnd::Switch_ShowMapLegend()
{
	m_map_legend_wnd->Show( !m_map_legend_wnd->IsShown() );
}

void CUITaskWnd::OnShowTreasures(CUIWindow* ui, void* d)
{
	m_bTreasuresEnabled = !m_bTreasuresEnabled;
	ReloadTaskInfo();
}
void CUITaskWnd::OnShowPrimaryObjects(CUIWindow* ui, void* d)
{
	m_bPrimaryObjectsEnabled = !m_bPrimaryObjectsEnabled;
	ReloadTaskInfo();
}
void CUITaskWnd::OnShowSecondaryTasks(CUIWindow* ui, void* d)
{
	m_bSecondaryTasksEnabled = !m_bSecondaryTasksEnabled ;
	ReloadTaskInfo();
}
void CUITaskWnd::OnShowQuestNpcs(CUIWindow* ui, void* d)
{
	m_bQuestNpcsEnabled = !m_bQuestNpcsEnabled;
	ReloadTaskInfo();
}
// --------------------------------------------------------------------------------------------------
CUITaskItem::CUITaskItem() : m_owner(nullptr), show_hint_can(false), show_hint(false), m_hint_wt(500) {}

CUIStatic* init_static_field(CUIXml& uiXml, LPCSTR path, LPCSTR path2);

void CUITaskItem::Init(CUIXml& uiXml, LPCSTR path)
{
	CUIXmlInit::InitWindow			(uiXml,path,0,this);
	m_hint_wt						= uiXml.ReadAttribInt(path, 0, "hint_wt", 500);

	string256		buff;
	CUIStatic* S					= nullptr;

	xr_strconcat(buff, path, ":t_icon" );
	if ( uiXml.NavigateToNode( buff ) )
	{
		S = init_static_field		(uiXml, path, "t_icon");
		AttachChild					(S);
	}
	m_info["t_icon"]				= S;
	
	xr_strconcat(buff, path, ":t_icon_over" );
	if ( uiXml.NavigateToNode( buff ) )
	{
		S = init_static_field		(uiXml, path, "t_icon_over");
		AttachChild					(S);
	}
	m_info["t_icon_over"]			= S;
	
	S = init_static_field			(uiXml, path, "t_caption");
	AttachChild						(S);
	m_info["t_caption"]				= S;

	show_hint_can = false;
	show_hint     = false;
}

void CUITaskItem::InitTask(CGameTask* task)
{
	m_owner							= task;
	CUIStatic* S					= m_info["t_icon"];
	if ( S )
	{
		if ( task )
		{
			S->InitTexture			(task->m_icon_texture_name.c_str());
			S->TextureOn();
			S->SetStretchTexture	(true);
			m_info["t_icon_over"]->Show(true);
		}
		else
		{
			S->TextureOff			();
			m_info["t_icon_over"]->Show(false);
		}
	}

	S								= m_info["t_caption"];
	S->TextItemControl()->SetTextST	((task) ? task->m_Title.c_str() : "");
}

void CUITaskItem::OnFocusReceive()
{
	inherited::OnFocusReceive();
	show_hint_can = true;
	show_hint     = false;
}

void CUITaskItem::OnFocusLost()
{
	inherited::OnFocusLost();
	show_hint_can = false;
	show_hint     = false;
}

void CUITaskItem::Update()
{
	inherited::Update();
	if ( m_owner && m_bCursorOverWindow && show_hint_can )
	{
		if ( Device.dwTimeGlobal > ( m_dwFocusReceiveTime + m_hint_wt ) )
		{
			show_hint = true;
			return;
		}
	}
}

void CUITaskItem::OnMouseScroll( float iDirection )
{
}

bool CUITaskItem::OnMouseAction( float x, float y, EUIMessages mouse_action )
{
	if ( inherited::OnMouseAction( x, y, mouse_action ) )
	{
		//return true;
	}

	switch ( mouse_action )
	{
	case WINDOW_LBUTTON_DOWN:
	case WINDOW_RBUTTON_DOWN:
	case BUTTON_DOWN:
		show_hint_can = false;
		show_hint     = false;
		break;
	}//switch

	return true;
}

void CUITaskItem::SendMessage( CUIWindow* pWnd, s16 msg, void* pData )
{
	inherited::SendMessage( pWnd, msg, pData );
}
