#include "stdafx.h"

#define STB_IMAGE_IMPLEMENTATION
#include <SDL_Ext/SDL_image.h>
#include <SDL3/SDL.h>

#include "Splash.h"
#define ErrorMsg(fmt,...) Msg("[Editor Splash]<%s ~ line %d>" fmt, __FUNCTION__, __LINE__, __VA_ARGS__)

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#pragma warning(disable: 4047)
HINSTANCE hInstanceG = (HINSTANCE)&__ImageBase;
#pragma warning(default: 4047)

SDL_Window* splashWindow = nullptr;
SDL_Renderer* splashRenderer = nullptr;
SDL_Texture* texture = nullptr;
bool isInit = false;
int WinW = 0, WinH = 0;

void Update(int progress = 0)
{
	int pgHeight = 10;

	SDL_FRect progressBarBackground = { 0, WinH - pgHeight, WinW, pgHeight };
	SDL_FRect progressBarFill = 
	{ 
		progressBarBackground.x, 
		progressBarBackground.y,
		(progress * progressBarBackground.w) / 100,
		progressBarBackground.h
	};

	SDL_RenderClear(splashRenderer);

	SDL_QueryTexture(texture, nullptr, nullptr, &WinW, &WinH);

	SDL_FRect dstRect = { 0, 0, WinW, WinH };
	SDL_RenderTexture(splashRenderer, texture, nullptr, &dstRect);

	SDL_SetRenderDrawColor(splashRenderer, 150, 150, 150, 255);
	SDL_RenderFillRect(splashRenderer, &progressBarBackground);

	SDL_SetRenderDrawColor(splashRenderer, 3, 181, 3, 255);
	SDL_RenderFillRect(splashRenderer, &progressBarFill);

	SDL_RenderPresent(splashRenderer);
}

SDL_Surface* LoadPNGSurfaceFromResource(unsigned char* imageData, LPCTSTR lpName, LPCTSTR lpType) {
	HMODULE hMODULE = hInstanceG;

	HRSRC hRes = FindResource(hMODULE, lpName, lpType);
	if (!hRes) {
		ErrorMsg("Failed to find resource (ID %d)", lpName);
		return nullptr;
	}

	HGLOBAL hMem = LoadResource(hInstanceG, hRes);
	if (!hMem) {
		ErrorMsg("Failed to load resource (ID %d)", lpName);
		return nullptr;
	}

	void* pResData = LockResource(hMem);
	if (!pResData) {
		ErrorMsg("Failed to lock resource (ID %d)", lpName);
		return nullptr;
	}

	DWORD resSize = SizeofResource(hInstanceG, hRes);

	int width, height, channels;
	imageData = stbi_load_from_memory((unsigned char*)pResData, resSize, &width, &height, &channels, STBI_rgb_alpha);
	if (!imageData) {
		ErrorMsg("Failed to decode PNG (ID %d)", lpName);
		return nullptr;
	}
	
	SDL_Surface* surface = SDL_CreateSurfaceFrom(
		imageData,            
		width,                
		height,               
		width * 4,            
		SDL_PIXELFORMAT_RGBA32
	);

	if (!surface) {
		stbi_image_free(imageData);
		ErrorMsg("Failed to create pixel format (ID %d). %s", lpName, SDL_GetError());
		return nullptr;
	}

	WinW = width;
	WinH = height;

	return surface;
}

void Destroy()
{
	SDL_DestroyRenderer(splashRenderer);
	SDL_DestroyWindow(splashWindow);
	SDL_DestroyTexture(texture);

	splashRenderer = nullptr;
	splashWindow = nullptr;
	texture = nullptr;
}

namespace splash
{
	ECORE_API void show(int idb)
	{
		if (isInit) return;

		if (SDL_Init(0) != 0) {
			ErrorMsg("SDL_Init Error: %s", SDL_GetError());
			return;
		}
		
		unsigned char* imageData = nullptr;

		SDL_Surface* surface = LoadPNGSurfaceFromResource(imageData, MAKEINTRESOURCE(idb), _T("PNG"));
		
		if (!surface)
		{
			ErrorMsg("Failed to create surface (ID %d)", idb);
			return;
		}

		splashWindow = SDL_CreateWindow(
			"Loading...", 
			WinW,
			WinH,
			SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP | 
			SDL_WINDOW_NOT_FOCUSABLE | SDL_WINDOW_TRANSPARENT
		);
		
		if (!splashWindow) {
			Destroy();
			ErrorMsg("SDL_CreateWindow Error: %s", SDL_GetError());
			return;
		}

		splashRenderer = SDL_CreateRenderer(splashWindow, NULL, NULL
		/*"splashRenderer", SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC*/);
		if (!splashRenderer) {
			Destroy();
			ErrorMsg("SDL_CreateRenderer Error: %s", SDL_GetError());
			return;
		}
		
		texture = SDL_CreateTextureFromSurface(splashRenderer, surface);
		SDL_DestroySurface(surface);
		stbi_image_free(imageData);

		if (!texture) {
			ErrorMsg("Failed to create texture (ID %d)", idb);
			return;
		}

		Update();

		isInit = true;
		return;
	}

	ECORE_API void splash::update(int progress, const char[])
	{
		if (!isInit) return;
		
		Update(progress);

		return;
	}

	ECORE_API void hide()
	{
		Destroy();
	}
}