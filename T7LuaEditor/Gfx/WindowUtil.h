#ifndef WINDOW_UTIL_H
#define WINDOW_UTIL_H


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <fmt/format.h> // for logging
#include <assert.h>
#include <imgui_impl_win32.h>
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

namespace WindowUtil
{
    extern HWND g_mainHwnd;
    extern HINSTANCE g_hInst;
    extern std::wstring g_windowName;
    extern std::wstring g_windowClassName;

    HWND NewWindow(std::wstring windowName, HINSTANCE hInst, bool nCmdShow);
    void Cleanup();
    LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

}

#endif // !WINDOW_UTIL_H