	////////////////////////////////////////////////////////////////////////////
//	Module 		: UIInvUpgradeInfo.cpp
//	Created 	: 21.11.2007
//  Modified 	: 13.03.2009
//	Author		: Evgeniy Sokolov, Prishchepa Sergey
//	Description : inventory upgrade UI info window class implementation
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pch_script.h"
#include "UIInvUpgradeInfo.h"
#include "../../xrEngine/string_table.h"
#include "../Actor.h"

#include "../../xrUI/Widgets/UIStatic.h"
#include "../../xrUI/UIXmlInit.h"
#include "../../xrUI/Widgets/UIFrameWindow.h"

#include "UIInvUpgradeProperty.h"

#include "inventory_upgrade.h"
#include "inventory_upgrade_property.h"
#include "../../xrUI/UIHelper.h"


UIInvUpgradeInfo::UIInvUpgradeInfo()
{
	m_upgrade    = nullptr;
	m_background = nullptr;
	m_name       = nullptr;
	m_cost		 = nullptr;
	m_desc       = nullptr;
	m_prereq     = nullptr;
	m_properties_wnd = nullptr;
}

UIInvUpgradeInfo::~UIInvUpgradeInfo()
{
}

void UIInvUpgradeInfo::init_from_xml( LPCSTR xml_name )
{
	CUIXml ui_xml;
	ui_xml.Load( CONFIG_PATH, UI_PATH, xml_name );
	CUIXmlInit xml_init;
	
	XML_NODE* stored_root = ui_xml.GetLocalRoot();
	XML_NODE* node = ui_xml.NavigateToNode( "upgrade_info", 0 );
	ui_xml.SetLocalRoot( node );

	xml_init.InitWindow( ui_xml, "main_frame", 0, this );
	
	m_background = UIHelper::CreateFrameWindow(ui_xml, "background_frame", this);
	m_name = UIHelper::CreateTextWnd(ui_xml, "info_name", this);

	if (ui_xml.NavigateToNode("info_cost"))
	m_cost = UIHelper::CreateTextWnd(ui_xml, "info_cost", this);

	m_desc = UIHelper::CreateTextWnd(ui_xml, "info_desc", this);
	m_prereq = UIHelper::CreateTextWnd(ui_xml, "info_prerequisites", this);

	m_properties_wnd = new UIInvUpgPropertiesWnd();	 
	AttachChild( m_properties_wnd );
	m_properties_wnd->SetAutoDelete( true );
	m_properties_wnd->init_from_xml( xml_name );
	
	m_properties_wnd->Show( false );
	ui_xml.SetLocalRoot( stored_root );
}

bool UIInvUpgradeInfo::init_upgrade( Upgrade_type* upgr, CInventoryItem* inv_item )
{
	if ( !upgr || !inv_item )
	{
		m_upgrade = nullptr;
		Show( false );
		return false;
	}
	
	if ( m_upgrade == upgr )
	{
		return false;
	}
	m_upgrade = upgr;

	Show( true );
	m_name->Show( true );
	m_desc->Show( true );

	m_name->SetText( m_upgrade->name() );
	m_desc->SetText( m_upgrade->description_text() );

	if (m_upgrade->is_known())
	{
		m_desc->SetText(m_upgrade->description_text());
		m_prereq->Show(true);

		inventory::upgrade::UpgradeStateResult upg_res = m_upgrade->can_install(*inv_item, false);
		if (upg_res == inventory::upgrade::result_ok || upg_res == inventory::upgrade::result_e_precondition_money
			|| upg_res == inventory::upgrade::result_e_precondition_quest)
		{
			m_prereq->SetText(m_upgrade->get_prerequisites());
		}
		else
		{
			string32 str_res;
			switch (upg_res)
			{
			case inventory::upgrade::result_e_unknown:
				xr_strcpy(str_res, sizeof(str_res), "st_upgr_unknown");
				break;
			case inventory::upgrade::result_e_installed:
				xr_strcpy(str_res, sizeof(str_res), "st_upgr_installed");
				break;
			case inventory::upgrade::result_e_parents:
				xr_strcpy(str_res, sizeof(str_res), "st_upgr_parents");
				break;
			case inventory::upgrade::result_e_group:
				xr_strcpy(str_res, sizeof(str_res), "st_upgr_group");
				break;
				//case inventory::upgrade::result_e_precondition:
			default:
				xr_strcpy(str_res, sizeof(str_res), "st_upgr_unknown");
				break;
			}
			m_prereq->SetTextST(str_res);
		}
		m_properties_wnd->Show(true);
	}

	if ( m_upgrade->is_known() )
	{
		m_prereq->Show( true );
		m_properties_wnd->Show( true );
		if (m_cost)
		{
			luabind::functor<LPCSTR> cost_func;
			LPCSTR cost_func_str = "inventory_upgrades.get_upgrade_cost";
			R_ASSERT2(ai().script_engine().functor(cost_func_str, cost_func), "Failed to get cost");
			m_cost->SetText(cost_func(m_upgrade->section()));
			m_cost->Show(true);
		}

		inventory::upgrade::UpgradeStateResult upg_res = m_upgrade->can_install( *inv_item, false );
		inventory::upgrade::UpgradeStateResult upg_res_script = m_upgrade->get_preconditions();
		string512 str_res = "";
		m_prereq->SetTextColor(color_rgba(255,90,90,255));
		if(upg_res==inventory::upgrade::result_e_installed)
		{
			m_prereq->SetTextColor(color_rgba(117,255,123,255));
			xr_sprintf(str_res, sizeof(str_res), "%s", g_pStringTable->translate("st_upgr_installed").c_str());
		}
		else if(upg_res==inventory::upgrade::result_e_unknown)
		{
			xr_sprintf(str_res, sizeof(str_res), "%s:\\n - %s", g_pStringTable->translate("st_upgr_disable").c_str(), g_pStringTable->translate("st_upgr_unknown").c_str());
			if (m_cost)
				m_cost->Show(false);
		}
		else if(upg_res==inventory::upgrade::result_e_group)
			xr_sprintf(str_res, sizeof(str_res), "%s:\\n - %s", g_pStringTable->translate("st_upgr_disable").c_str(), g_pStringTable->translate("st_upgr_group").c_str());
		else if(upg_res_script==inventory::upgrade::result_e_precondition_money)
			xr_sprintf(str_res, sizeof(str_res), "%s:\\n - %s", g_pStringTable->translate("st_upgr_disable").c_str(), g_pStringTable->translate("st_upgr_cant_do").c_str());
		else
		{
			if(upg_res!=inventory::upgrade::result_ok)
			{
				xr_sprintf(str_res, sizeof(str_res), "%s:\\n%s", g_pStringTable->translate("st_upgr_disable").c_str(), m_upgrade->get_prerequisites());
				if(upg_res==inventory::upgrade::result_e_parents)
					xr_sprintf(str_res, sizeof(str_res), "%s\\n - %s", str_res, g_pStringTable->translate("st_upgr_parents").c_str());

				if(upg_res==inventory::upgrade::result_e_precondition_money)
					xr_sprintf(str_res, sizeof(str_res), "%s:\\n - %s", g_pStringTable->translate("st_upgr_disable").c_str(), g_pStringTable->translate("st_upgr_cant_do").c_str());
			}
		}
		m_prereq->SetText(str_res);
	}
	else
	{
		m_desc->SetTextST("st_desc_unknown");
		m_prereq->Show( false );
		m_properties_wnd->Show( false );
		if (m_cost)
			m_cost->Show(false);
	}
	m_name->AdjustHeightToText();
	if (m_cost)
		m_cost->AdjustHeightToText();
	m_desc->AdjustHeightToText();
	m_prereq->AdjustHeightToText();

	Fvector2 new_pos;
	if (m_cost)
	{
		new_pos.x = m_cost->GetWndPos().x;
		new_pos.y = m_name->GetWndPos().y + m_name->GetWndSize().y + 5.0f;
		m_cost->SetWndPos(new_pos);

		new_pos.x = m_desc->GetWndPos().x;
		new_pos.y = m_cost->GetWndPos().y + m_cost->GetWndSize().y + 5.0f;
		m_desc->SetWndPos(new_pos);
	}
	else
	{
		new_pos.x = m_desc->GetWndPos().x;
		new_pos.y = m_name->GetWndPos().y + m_name->GetWndSize().y + 5.0f;
		m_desc->SetWndPos(new_pos);
	}
	new_pos.x = m_prereq->GetWndPos().x;
	new_pos.y = m_desc->GetWndPos().y + m_desc->GetWndSize().y + 5.0f;
	m_prereq->SetWndPos( new_pos );

	new_pos.x = m_properties_wnd->GetWndPos().x;
	new_pos.y = m_prereq->GetWndPos().y + m_prereq->GetWndSize().y + 5.0f;
	m_properties_wnd->SetWndPos( new_pos );
	
	m_properties_wnd->set_upgrade_info( *m_upgrade );

	// this wnd
	Fvector2					new_size;
	new_size.x					= GetWndSize().x;
	new_size.y					= m_properties_wnd->GetWndPos().y + m_properties_wnd->GetWndSize().y + 10.0f;
	SetWndSize					(new_size);
	m_background->SetWndSize	(new_size);
	
	return true;
}

void UIInvUpgradeInfo::Draw()
{
	if ( m_upgrade )
	{
		inherited::Draw();
	}
}
