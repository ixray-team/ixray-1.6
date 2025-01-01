// LevelEditor.cpp : Определяет точку входа для приложения.
//
#include "stdafx.h"
#include "Engine/XrGameManager.h"
#include "..\xrEngine\std_classes.h"
#include "..\xrEngine\IGame_Persistent.h"
#include "..\xrEngine\XR_IOConsole.h"
#include "..\xrEngine\IGame_Level.h"
#include "..\xrEngine/string_table.h"
#include "..\xrEngine\x_ray.h"
#include "Engine/XRayEditor.h"
#include "../../xrEngine/xr_input.h"
#include "Editor/Utils/ContentView.h"
#include "xrECore/Splash.h"

ECORE_API extern bool bIsLevelEditor;
void DragDrop(const xr_string&, int);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	bIsLevelEditor = true;

	//if (strstr(GetCommandLine(), "-nosplash") == nullptr)
	{
		constexpr bool topmost = true;
		splash::show(topmost);
	}

	splash::update_progress(1);

	if (!IsDebuggerPresent())
		Debug._initialize(false);
	
	splash::update_progress(5);

	const char* FSName = "fs.ltx";
	Core._initialize("LevelEditor", ELogCallback, 1, FSName);

	splash::update_progress(24);

	Tools = new CLevelTool();
	LTools = static_cast<CLevelTool*>(Tools);

	splash::update_progress(5);

	UI = new CLevelMain();
	UI->RegisterCommands();

	LUI = static_cast<CLevelMain*>(UI);

	splash::update_progress(15);

	Scene = new EScene();
	EditorScene = Scene;
	GContentView = new CContentView;
	UIMainForm* MainForm = new UIMainForm();
	pApp = new XRayEditor();
	g_pStringTable = new CStringTable();
	g_XrGameManager = new XrGameManager();
	g_SEFactoryManager = new XrSEFactoryManager();

	splash::update_progress(24);

	// Initialize APP
	GameMaterialLibraryEditors->Load();

	g_pGamePersistent = static_cast<IGame_Persistent*>(g_XrGameManager->Create(CLSID_GAME_PERSISTANT));
	EDevice->seqAppStart.Process(rp_AppStart);

	Console->Execute("default_controls");

	xr_strcpy(Console->ConfigFile, "user.ltx");

	if (strstr(Core.Params, "-ltx ")) {
		string64 c_name;
		sscanf(strstr(Core.Params, "-ltx ") + 5, "%[^ ] ", c_name);
		xr_strcpy(Console->ConfigFile, c_name);
	}

	Console->ExecuteScript(Console->ConfigFile);

	Console->Hide();

	::MainForm = MainForm;
	UI->Push(MainForm, false);
	
	splash::update_progress(25);
	
	splash::update_progress(1);

	bool NeedExit = false;
	MainForm->GetRenderForm()->DragFunctor = DragDrop;
	
	GContentView->Init();
	UI->PushBegin(GContentView);

	splash::hide();
	while (!NeedExit)
	{
		SDL_Event Event;
		while (SDL_PollEvent(&Event))
		{
			switch (Event.type)
			{
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				//EPrefs->SaveConfig();
				GContentView->Destroy();
				NeedExit = true;
				break;

			case SDL_EVENT_WINDOW_RESIZED:
				if (UI && REDevice)
				{
					UI->Resize(Event.window.data1, Event.window.data2, true);
					EPrefs->SaveConfig();
				}
				break;
			case SDL_EVENT_WINDOW_SHOWN:
			case SDL_EVENT_WINDOW_MOUSE_ENTER:
				Device.b_is_Active = true;
				//if (UI) UI->OnAppActivate();

				break;
			case SDL_EVENT_WINDOW_HIDDEN:
			case SDL_EVENT_WINDOW_MOUSE_LEAVE:
				Device.b_is_Active = !!psDeviceFlags.test(rsDeviceActive);
				//if (UI)UI->OnAppDeactivate();
				break;

			case SDL_EVENT_KEY_DOWN:
				if (UI)
				{
					UI->KeyDown(Event.key.keysym.scancode, UI->GetShiftState());
					UI->ApplyShortCutInput(Event.key.keysym.scancode);

					if (UI->IsPlayInEditor())
					{
						if (pInput->IsAcquire)
						{
							pInput->KeyboardButtonUpdate(Event.key.keysym.scancode, true);
						}
						else if (Event.key.keysym.scancode == SDL_SCANCODE_LALT)
						{
							pInput->acquire();
							UI->IsEnableInput = false;
							ShowCursor(FALSE);
						}
					}
				}break;
			case SDL_EVENT_KEY_UP:
				if (UI) {
					UI->KeyUp(Event.key.keysym.scancode, UI->GetShiftState());
					if(UI->IsPlayInEditor() && pInput->IsAcquire) 
					{
						if (pInput->IsAcquire)
						{
							pInput->KeyboardButtonUpdate(Event.key.keysym.scancode, false);
						}
					}
				}
				break;
			case SDL_EVENT_MOUSE_MOTION:
			{
				if (UI->IsPlayInEditor() && !pInput->IsAcquire)
					break;

				pInput->MouseMotion(Event.motion.xrel, Event.motion.yrel);
			} break;
			case SDL_EVENT_MOUSE_WHEEL:
			{
				if (UI->IsPlayInEditor() && !pInput->IsAcquire)
					break;

				pInput->MouseScroll(Event.wheel.y);
			}break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
			{
				if (UI->IsPlayInEditor() && !pInput->IsAcquire)
					break;

				int mouse_button = 0;
				if (Event.button.button == SDL_BUTTON_LEFT) { mouse_button = 0; }
				if (Event.button.button == SDL_BUTTON_RIGHT) { mouse_button = 1; }
				if (Event.button.button == SDL_BUTTON_MIDDLE) { mouse_button = 2; }
				if (Event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
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

	xr_delete(g_FontManager);
	xr_delete(MainForm);
	xr_delete(pApp);
	xr_delete(g_XrGameManager);
	xr_delete(g_SEFactoryManager);

	Core._destroy();
	return 0;
}