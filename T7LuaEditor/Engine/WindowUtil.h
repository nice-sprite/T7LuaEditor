#ifndef WINDOW_UTIL_H
#define WINDOW_UTIL_H

#include "../t7pch.h"
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

// class Window
// {
// private:
//     static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

//     static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

//     LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

// public:
//     Window(HINSTANCE hInst, std::wstring _title, std::wstring _classname, int _width, int _height);

//     bool ProcessMessages();
//     // only because you *really* shouldn't change these.
//     [[nodiscard]] std::wstring GetTitle() const;

//     [[nodiscard]] std::wstring GetClassname() const;

//     [[nodiscard]] HWND GetHwnd() const { return hwnd; }

//     ~Window()
//     {
//         DestroyWindow(hwnd);
//         UnregisterClass(classname.c_str(), hInst);
//     }

//     std::function < LRESULT()

//                         private : HWND hwnd;
//     HINSTANCE hInst;
//     int width, height;
//     std::wstring title;
//     std::wstring classname;

//     Mouse mouse;
//     Keyboard keyboard;
// };

#endif // !WINDOW_UTIL_H