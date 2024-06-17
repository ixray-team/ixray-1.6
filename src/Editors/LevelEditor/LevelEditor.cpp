﻿// LevelEditor.cpp : Определяет точку входа для приложения.
//
#include "stdafx.h"
#include "Engine/XrGameManager.h"
#include "..\XrEngine\std_classes.h"
#include "..\XrEngine\IGame_Persistent.h"
#include "..\XrEngine\XR_IOConsole.h"
#include "..\XrEngine\IGame_Level.h"
#include "..\XrEngine\x_ray.h"
#include "Engine/XRayEditor.h"
#include "../../xrEngine/xr_input.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    if (!IsDebuggerPresent())
        Debug._initialize(false);
    
    const char* FSName = "fs.ltx";
    Core._initialize("LevelEditor", ELogCallback, 1, FSName);
    Tools = xr_new<CLevelTool>();
    LTools = static_cast<CLevelTool*>(Tools);

    UI = xr_new<CLevelMain>();
    UI->RegisterCommands();
    LUI = static_cast<CLevelMain*>(UI);

    Scene = xr_new<EScene>();
    EditorScene = Scene;
    UIMainForm* MainForm = xr_new< UIMainForm>();
    pApp = xr_new<XRayEditor>();
    g_XrGameManager = xr_new<XrGameManager>();
    g_SEFactoryManager = xr_new<XrSEFactoryManager>();

    // Initialize APP
	g_FontManager = new CFontManager();
	g_FontManager->InitializeFonts();
    GameMaterialLibraryEditors->Load();

    g_pGamePersistent = static_cast<IGame_Persistent*>(g_XrGameManager->Create(CLSID_GAME_PERSISTANT));
    EDevice->seqAppStart.Process(rp_AppStart);
    Console->Execute("default_controls");
    Console->Hide();

    ::MainForm = MainForm;
    UI->Push(MainForm, false);
    bool NeedExit = false;

    while (!NeedExit)
    {
        SDL_Event Event;
        while (SDL_PollEvent(&Event))
        {
            switch (Event.type)
            {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                EPrefs->SaveConfig();
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
                Device.b_is_Active = false;
                //if (UI)UI->OnAppDeactivate();
                break;

            case SDL_EVENT_KEY_DOWN:
                if (UI)
                {
                    UI->KeyDown(Event.key.keysym.scancode, UI->GetShiftState());
                    UI->ApplyShortCutInput(Event.key.keysym.scancode);
                }break;
            case SDL_EVENT_KEY_UP:
                if (UI)UI->KeyUp(Event.key.keysym.scancode, UI->GetShiftState());
                break;

            case SDL_EVENT_MOUSE_MOTION:
                pInput->MouseMotion(Event.motion.xrel, Event.motion.yrel);
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                pInput->MouseScroll(Event.wheel.y);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
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

	xr_delete(MainForm);
	xr_delete(pApp);
	xr_delete(g_XrGameManager);
	xr_delete(g_SEFactoryManager);
	xr_delete(g_FontManager);

    Core._destroy();
    return 0;
}
