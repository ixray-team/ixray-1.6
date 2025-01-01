#include "stdafx.h"
#include "resource.h"
#include "Splash.h"
#include <CommCtrl.h>
//---------------------------------------------------------------------------

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#pragma warning(disable: 4047)
HINSTANCE hInstanceG = (HINSTANCE)&__ImageBase;
#pragma warning(default: 4047)

HINSTANCE               hInst;                        // Указатель на экземпляр приложения.
HWND                    hProgress;                    // Дескриптор прогресс бара.
int                     nCounter;                     // Счётчик.
COLORREF                clrBg = RGB(25, 25, 0);   // Цвет фона прогресс бара.
COLORREF                clrBarRed = RGB(255, 0, 0);   // Цвет индикатора хода выполнения прогресс бара.
// COLORREF  clrBarYellow = RGB(255, 255, 0); // Цвет индикатора хода выполнения прогресс бара.
// COLORREF  clrBarGreen  = RGB(0, 255, 0);   // Цвет индикатора хода выполнения прогресс бара.

HBITMAP                 hSplashBMP;
HWND                    logoWindow = nullptr;

int                     m_MinPos = 0;
int                     m_MaxPos = 100;

static INT_PTR CALLBACK LogoWndProc(HWND hw, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        char        buffer[64];
        RECT        rect;
        PAINTSTRUCT ps;
        HDC         hdc = BeginPaint(hw, &ps);
        HBRUSH      brush = CreatePatternBrush(hSplashBMP);

        GetClientRect(hw, &rect);
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);

        rect.top = 275;
        // Текст и роценты процесса стадий загрузки.
        if (nCounter <= 1)
            sprintf_s(buffer, "Start Load...  %d %%", nCounter);
        else if (nCounter <= 6)
            sprintf_s(buffer, "Core initializing...  %d %%", nCounter);
        else if (nCounter <= 30)
            sprintf_s(buffer, "Loading...  %d %%", nCounter);
        else if (nCounter <= 35)
            sprintf_s(buffer, "Register Commands...  %d %%", nCounter);
        else if (nCounter <= 50)
            sprintf_s(buffer, "Loading Editor...  %d %%", nCounter);
        else if (nCounter <= 74)
            sprintf_s(buffer, "Loading MainForm...  %d %%", nCounter);
        else if (nCounter <= 99)
            sprintf_s(buffer, "Loading...  %d %%", nCounter);
        else if (nCounter == 100)
            sprintf_s(buffer, "Loading is Complete!  %d %%", nCounter);

        // Устанавливаем Цвет текста.
        if (nCounter == 100)
            SetTextColor(hdc, RGB(0x00, 0xFF, 0x00));
        else
            SetTextColor(hdc, RGB(0xFF, 0x00, 0x00));
        SetBkMode(hdc, TRANSPARENT);
        DrawTextA(hdc, buffer, -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_CENTER);
        EndPaint(hw, &ps);
    }
    break;
    case WM_INITDIALOG:
    {
        // Здесь мы должны использовать исполняемый модуль для загрузки нашего растрового ресурса.
        hSplashBMP = LoadBitmap(hInstanceG, MAKEINTRESOURCE(IDB_BITMAP1));

        hProgress = CreateWindowEx(0,
            PROGRESS_CLASS,   // Указатель на класс прогресс бара
            (LPTSTR)NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH /* | WS_BORDER*/,
            (600-370)/2,    // X
            322-30,   // Y
            370,   // Ширина
            13,    // Высота
            hw, (HMENU)0, hInstanceG, NULL);
        // Устанавливаем Цвета прогресс бара.
        SendMessage(hProgress, PBM_SETBKCOLOR, 0, (LPARAM)clrBg);
        SendMessage(hProgress, PBM_SETBARCOLOR, 0, (LPARAM)clrBarRed);
        // Устанавливаем диапазон прогресс бара.
        SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(m_MinPos, m_MaxPos));
        SendMessage(hProgress, PBM_SETSTEP, (WPARAM)1, 0);
        nCounter = 0;
    }
    break;
    case WM_DESTROY:
        break;
    case WM_CLOSE:
        DestroyWindow(hw);
        break;
    case WM_COMMAND:
        if (LOWORD(wp) == IDCANCEL)
            DestroyWindow(hw);
        break;
    default:
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------

namespace splash
{
    ECORE_API void show(const bool topmost)
    {
        if (logoWindow)
            return;

        //logoWindow = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_STARTUP), nullptr, LogoWndProc);
        logoWindow = CreateDialog(hInstanceG, MAKEINTRESOURCE(IDD_STARTUP), nullptr, LogoWndProc);
        const HWND logoPicture = GetDlgItem(logoWindow, IDC_STATIC);
        RECT       logoRect;
        GetWindowRect(logoPicture, &logoRect);
        const HWND prevWindow = topmost ? HWND_TOPMOST : HWND_NOTOPMOST;
        SetWindowPos(logoWindow, HWND_TOPMOST, 0, 0, logoRect.right - logoRect.left, logoRect.bottom - logoRect.top, SWP_NOMOVE | SWP_SHOWWINDOW);
        UpdateWindow(logoWindow);
    }

    ECORE_API void update_progress(int progress)
    {
        nCounter += progress;
        clamp(nCounter, m_MinPos, m_MaxPos);
        SendMessage(hProgress, PBM_SETPOS, nCounter, 0);
        RedrawWindow(logoWindow, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }

    ECORE_API void hide()
    {
        if (logoWindow != nullptr)
        {
            DestroyWindow(logoWindow);
            logoWindow = nullptr;
        }
    }
}   // namespace splash

//---------------------------------------------------------------------------
