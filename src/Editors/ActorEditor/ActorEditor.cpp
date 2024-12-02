// ActorEditor.cpp : Определяет точку входа для приложения.
//
#include "stdafx.h"
#include "../../xrEngine/xr_input.h"

#include "../xrEProps/UIBoneView.h"

void DragFile(xr_string File)
{
	bool NeedConv = IsUTF8(File.c_str());
	File = Platform::UTF8_to_CP1251(File);

	CActorTools* pTools = (CActorTools*)Tools;
	auto pObject = pTools->CurrentObject();

	auto ErrorMsg = []()
	{
		SDL_MessageBoxButtonData BtnOk = {};
		BtnOk.text = "Ok";

		SDL_MessageBoxData ErrorData = {};
		ErrorData.message = "Need load object!";
		ErrorData.title = "Error!";
		ErrorData.buttons = &BtnOk;
		ErrorData.numbuttons = 1;

		int RetBtn = 0;
		SDL_ShowMessageBox(&ErrorData, &RetBtn);
	};

	if (File.ends_with(".object"))
	{
		ExecCommand(COMMAND_LOAD, File);
	}
	else if (File.Contains(".skl"))
	{
		if (pObject != nullptr)
		{
			FS.TryLoad(File);
			pObject->AppendSMotion(File.c_str());
		}
		else
		{
			ErrorMsg();
		}
		ExecCommand(COMMAND_UPDATE_PROPERTIES);
	}
	else if (File.ends_with(".omf"))
	{
		if (pObject != nullptr)
		{
			string_path CurrentWD = {};
			string_path CurrentGM = {};
			FS.update_path(CurrentGM, "$game_meshes$", "");
			FS.update_path(CurrentWD, "$fs_root$", CurrentGM);

			if (File.Contains(CurrentWD))
			{
				pObject->m_SMotionRefs.emplace_back(File.substr(strlen(CurrentWD)).c_str());
			}
			else
			{
				xr_path FilePath = File;
				xr_string FileName = FilePath.xfilename();
				xr_string OutPath = CurrentWD + FileName;

				// Make temp file
				std::filesystem::copy_file(File.c_str(), OutPath.c_str());
				FS.TryLoad((CurrentGM + FileName).c_str());

				pObject->m_SMotionRefs.emplace_back(FileName.substr(0, FileName.length() - 4).c_str());
			}
			ExecCommand(COMMAND_UPDATE_PROPERTIES);
		}
		else
		{
			ErrorMsg();
		}
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	if (!IsDebuggerPresent())
		Debug._initialize(false);

	const char* FSName = "fs.ltx";
	Core._initialize("Actor", ELogCallback, 1, FSName);

	Tools = new CActorTools();
	ATools = (CActorTools*)Tools;
	UI = new CActorMain();
	UI->RegisterCommands();

	UIMainForm* MainForm = new UIMainForm();
	::MainForm = MainForm;

	PGMLib->Load();

	UI->PushBegin(MainForm, false);

	int ArgsCount = 0;
	auto Commands = CommandLineToArgvW(GetCommandLine(), &ArgsCount);

	if (ArgsCount > 1)
	{
		xr_string SecondArg = Platform::UTF8_to_CP1251(Platform::TCHAR_TO_ANSI_U8(Commands[1]));
		if (SecondArg.ends_with(".object"))
		{
			ExecCommand(COMMAND_LOAD, SecondArg);
		}
	}

	bool NeedExit = false;

	while (!NeedExit)
	{
		SDL_Event Event;
		while (SDL_PollEvent(&Event))
		{
			switch (Event.type)
			{
			case SDL_QUIT:
				EPrefs->SaveConfig();
				NeedExit = true;
				break;

			case SDL_WINDOWEVENT:
				switch (Event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
					if (UI && REDevice)
					{
						UI->Resize(Event.window.data1, Event.window.data2, true);
						EPrefs->SaveConfig();
					}
					break;
				case SDL_WINDOWEVENT_SHOWN:
				case SDL_WINDOWEVENT_ENTER:
					Device.b_is_Active = true;
					break;
				case SDL_WINDOWEVENT_HIDDEN:
				case SDL_WINDOWEVENT_LEAVE:
					Device.b_is_Active = false;
					break;
				}
				break;

			case SDL_KEYDOWN:
				if (UI)
				{
					UI->KeyDown(Event.key.keysym.scancode, UI->GetShiftState());
					UI->ApplyShortCutInput(Event.key.keysym.scancode);
				}
				break;

			case SDL_KEYUP:
				if (UI) UI->KeyUp(Event.key.keysym.scancode, UI->GetShiftState());
				break;

			case SDL_MOUSEMOTION:
				pInput->MouseMotion(Event.motion.xrel, Event.motion.yrel);
				break;

			case SDL_MOUSEWHEEL:
				pInput->MouseScroll(Event.wheel.y);
				break;

			case SDL_DROPFILE:
			{
				xr_string File = strlwr(Event.drop.file);
				DragFile(File);
				SDL_free(Event.drop.file);
				break;
			}

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				int mouse_button = 0;
				if (Event.button.button == SDL_BUTTON_LEFT) { mouse_button = 0; }
				if (Event.button.button == SDL_BUTTON_RIGHT) { mouse_button = 1; }
				if (Event.button.button == SDL_BUTTON_MIDDLE) { mouse_button = 2; }
				if (Event.type == SDL_MOUSEBUTTONDOWN) {
					pInput->MousePressed(mouse_button);
				}
				else {
					pInput->MouseReleased(mouse_button);
				}
			}
			break;
			}

			if (!UI->ProcessEvent(&Event))
				break;
		}
		MainForm->Frame();
	}

	xr_delete(MainForm);

	Core._destroy();
	return 0;
}