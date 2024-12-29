#include "stdafx.h"
#include "UIMacroView.h"

#include "../../xrEUI/ImNodeEditor/imnodes.h"

#include <magic_enum/magic_enum.hpp>

std::array<size_t, 21> CommandsOneArgs =
{
	COMMAND_CHANGE_ACTION,
	COMMAND_CHANGE_AXIS,
	COMMAND_ICON_PICKER,
	COMMAND_SOUND_EDITOR,
	COMMAND_SHOW_PROPERTIES,
	COMMAND_UPDATE_PROPERTIES,
	COMMAND_ZOOM_EXTENTS,
	COMMAND_GRID_NUMBER_OF_SLOTS,
	COMMAND_GRID_SLOT_SIZE,
	COMMAND_MUTE_SOUND,
	COMMAND_EXECUTE_COMMAND_LIST,
	COMMAND_LOG_COMMANDS,
	COMMAND_ICON_REMOVE,
	COMMAND_LOAD,
	COMMAND_UNLOAD_LEVEL_PART,
	COMMAND_LOAD_LEVEL_PART,
	COMMAND_HIDE_ALL,
	COMMAND_HIDE_SEL,
	COMMAND_LOCK_ALL,
	COMMAND_LOCK_SEL,
	COMMAND_SHOWCONTEXTMENU
};

std::array<size_t, 9> CommandsTwoArgs =
{
	COMMAND_SET_SETTINGS,
	COMMAND_IMAGE_EDITOR_SELECT,
	COMMAND_RUN_MACRO,
	COMMAND_ASSIGN_MACRO,
	COMMAND_CHANGE_TARGET,
	COMMAND_ENABLE_TARGET,
	COMMAND_SHOW_TARGET,
	COMMAND_READONLY_TARGET,
	COMMAND_SAVE
};


CUIMacroView::CUIMacroView()
{
	for (ECoreCommands StartCommand = (ECoreCommands)(COMMAND_INITIALIZE + 1); StartCommand < COMMAND_MAIN_LAST;)
	{
		xr_string CommandStr = magic_enum::enum_name(StartCommand).data();
		CommandStr = CommandStr.substr(xr_strlen("COMMAND_"));

		MacroNodeTypes NodeInst;
		NodeInst.CommandID = StartCommand;
		NodeInst.RegName = CommandStr;

		bool HasOneArg = std::find(CommandsOneArgs.begin(), CommandsOneArgs.end(), StartCommand) != CommandsOneArgs.end();
		bool HasTwoArg = std::find(CommandsTwoArgs.begin(), CommandsTwoArgs.end(), StartCommand) != CommandsTwoArgs.end();
		NodeInst.Type = HasOneArg ? MacroType::eOneArg : (HasTwoArg ? MacroType::eTwoArg : MacroType::eVoid);

		CommandsList.push_back(std::move(NodeInst));

		(*(u32*)&StartCommand)++;
	}

	for (ELECommand StartCommand = (ELECommand)(COMMAND_EXTFIRST_EXT + 1); StartCommand < COMMAND_LE_END;)
	{
		xr_string CommandStr = magic_enum::enum_name(StartCommand).data();
		CommandStr = CommandStr.substr(xr_strlen("COMMAND_"));
		
		MacroNodeTypes NodeInst;
		NodeInst.CommandID = StartCommand;
		NodeInst.RegName = CommandStr;

		bool HasOneArg = std::find(CommandsOneArgs.begin(), CommandsOneArgs.end(), StartCommand) != CommandsOneArgs.end();
		bool HasTwoArg = std::find(CommandsTwoArgs.begin(), CommandsTwoArgs.end(), StartCommand) != CommandsTwoArgs.end();
		NodeInst.Type = HasOneArg ? MacroType::eOneArg : (HasTwoArg ? MacroType::eTwoArg : MacroType::eVoid);

		CommandsList.push_back(std::move(NodeInst));
		(*(u32*)&StartCommand)++;
	}

	std::sort(CommandsList.begin(), CommandsList.end(), [](const MacroNodeTypes& Left, const MacroNodeTypes& Right)
		{
			return Left.RegName[0] < Right.RegName[0];
		});
}

CUIMacroView::~CUIMacroView()
{
	for (auto Node : Nodes)
	{
		xr_delete(Node);
	}
}

void CUIMacroView::Draw()
{
	if (!IsOpen)
		return;

	if (ImGui::Begin("Macro Node Viewer", &IsOpen))
	{
		auto& io = ImGui::GetIO();
		ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

		ImGui::Separator();
		
		int HoveredNodeID = GetHoveredMode();

		if (ImGui::IsMouseReleased(1))
		{
			ImGui::OpenPopup("##nodesviewportcontextmenumacro");
		}

		if (ImGui::BeginPopup("##nodesviewportcontextmenumacro"))
		{
			if (ImGui::BeginMenu("Create Node"))
			{
				for (const MacroNodeTypes& Node : CommandsList)
				{
					if (ImGui::MenuItem(Node.RegName.data()))
					{
						const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

						CNodeMacro* MacroNode = (CNodeMacro*)Nodes.emplace_back(new CNodeMacro(Node.CommandID, Node.RegName));
						MacroNode->SetType(Node.Type);
						MacroNode->SetStartPos(click_pos.x, click_pos.y);
					}
				}

				ImGui::EndMenu();
			}

			if (HoveredNodeID != -1 && ImGui::MenuItem("Remove"))
			{
				Nodes.erase
				(
					std::find_if(Nodes.begin(), Nodes.end(), [HoveredNodeID](INodeUnknown* Val)
					{
						return Val->NodeID == HoveredNodeID;
					})
				);
			}

			ImGui::EndPopup();
		}
		CNodeViewport::Draw();
	}

	ImGui::End();

	CNodeViewport::DrawEnd();
}

void CUIMacroView::Show(bool State)
{
	IsOpen = State;
}