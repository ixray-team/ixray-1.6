#include "stdafx.h"

UITopBarForm::UITopBarForm()
{
    m_tUndo         = EDevice->Resources->_CreateTexture("ed\\bar\\Undo");
    m_timeUndo      = 0;
    m_tRedo         = EDevice->Resources->_CreateTexture("ed\\bar\\Redo");
    m_timeRedo      = 0;
    m_tSave         = EDevice->Resources->_CreateTexture("ed\\bar\\save");
    m_tReload       = EDevice->Resources->_CreateTexture("ed\\bar\\reload_configs");
    m_tOpenGameData = EDevice->Resources->_CreateTexture("ed\\bar\\open_gamedata");
}

UITopBarForm::~UITopBarForm() {}

void UITopBarForm::Draw()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + UI->GetMenuBarHeight()));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, UIToolBarSize));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = 0
        | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoSavedSettings
        ;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2( 2,2));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(2, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));
    ImGui::Begin("TOOLBAR", NULL, window_flags);
    {
        m_tUndo->Load();
        if (ImGui::ImageButton(m_tUndo->pSurface, ImVec2(20, 20), ImVec2(m_timeUndo > EDevice->TimerAsync() ? 0.5 : 0, 0), ImVec2(m_timeUndo > EDevice->TimerAsync() ? 1 : 0.5, 1), 0))
        {
            m_timeUndo = EDevice->TimerAsync() + 130;
            ClickUndo();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetTooltip("Undo the last action.");
        }
        ImGui::SameLine();

        m_tRedo->Load();
        if (ImGui::ImageButton(m_tRedo->pSurface, ImVec2(20, 20), ImVec2(m_timeRedo > EDevice->TimerAsync() ? 0.5 : 0, 0), ImVec2(m_timeRedo > EDevice->TimerAsync() ? 1 : 0.5, 1), 0))
        {
            m_timeRedo = EDevice->TimerAsync() + 130;
            ClickRedo();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetTooltip("Repeat the last action.");
        }
        ImGui::SameLine();

        m_tSave->Load();
        if (ImGui::ImageButton(m_tSave->pSurface, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0))
        {
            ClickSave();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetTooltip("Save all '.xr'");
        }
        ImGui::SameLine();

        m_tReload->Load();
        if (ImGui::ImageButton(m_tReload->pSurface, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0))
        {
            ClickReload();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetTooltip("Reload everything '.xr'");
        }
        ImGui::SameLine();

        m_tOpenGameData->Load();
        if (ImGui::ImageButton(m_tOpenGameData->pSurface, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0))
        {
            ClickOpenGameData();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetTooltip("Open folder 'GameData'");
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(5);
}

void UITopBarForm::ClickUndo()
{
    ExecCommand(COMMAND_UNDO);
}

void UITopBarForm::ClickRedo()
{
    ExecCommand(COMMAND_REDO);
}

void UITopBarForm::ClickSave()
{
    ExecCommand(COMMAND_SAVE);
}
void UITopBarForm::ClickReload()
{
    ExecCommand(COMMAND_LOAD);
}

void UITopBarForm::ClickOpenGameData()
{
    string_path GameDataPath;
    FS.update_path(GameDataPath, "$game_data$", "");
    ShellExecuteA(NULL, "open", GameDataPath, NULL, NULL, SW_SHOWDEFAULT);
}
