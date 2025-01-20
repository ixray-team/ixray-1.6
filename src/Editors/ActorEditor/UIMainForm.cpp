#include "stdafx.h"

#include "../xrECore/Editor/EditorChooseEvents.h"
UIMainForm* MainForm = nullptr;
UIMainForm::UIMainForm()
{
    EnableReceiveCommands();
    if (!ExecCommand(COMMAND_INITIALIZE, (u32)0, (u32)0)) 
    {
        xrLogger::FlushLog();
        exit(-1);
    }
    ExecCommand(COMMAND_UPDATE_GRID);
    ExecCommand(COMMAND_RENDER_FOCUS);

    FillChooseEvents();

    ToolBar = new CUIToolbar();
    ToolBar->OnCreate();

    m_TopBar = new UITopBarForm();
    m_Render = new UIRenderForm();
    m_MainMenu = new UIMainMenuForm();
    m_LeftBar = new UILeftBarForm();
    m_KeyForm = new UIKeyForm();
}

UIMainForm::~UIMainForm()
{
    ClearChooseEvents();
    xr_delete(m_KeyForm);
    xr_delete(m_LeftBar);
    xr_delete(m_MainMenu);
    xr_delete(m_Render);
    xr_delete(m_TopBar);
    xr_delete(ToolBar);

    ExecCommand(COMMAND_DESTROY, (u32)0, (u32)0);
}

void UIMainForm::Draw()
{
    m_MainMenu->Draw();
    m_TopBar->Draw();
    m_LeftBar->Draw();
    m_KeyForm->Draw();
   //ImGui::ShowDemoWindow(&bOpen);
    m_Render->Draw();
    ToolBar->Draw();
}

bool UIMainForm::Frame()
{
    if(UI)  return UI->Idle();
    return false;
}
