#ifndef WINDOW_UTIL_H
#define WINDOW_UTIL_H

#include "./Input.h"
#include "Renderer.h"
#include <functional>
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace WindowUtil
{
    struct Window
    {
        HWND hwnd;
        RECT clientRect;
    };

    Window Create(HINSTANCE hinst, const wchar_t *windowTitle,
                  const wchar_t *classname,
                  int width, int height, WNDPROC proc);

    void SetIcon(HWND hwnd, const wchar_t *iconPath);
};

#endif // !WINDOW_UTIL_H