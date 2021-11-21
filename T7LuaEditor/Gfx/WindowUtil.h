#ifndef WINDOW_UTIL_H
#define WINDOW_UTIL_H

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <string>
#include <fmt/format.h> // for logging
#include <cassert>
#include <imgui_impl_win32.h>
#include "Gfx.h"
#include <memory>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Window {
private:
    static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

    static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

    LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

public:
    Window(HINSTANCE hInst, std::wstring _title, std::wstring _classname, size_t _width, size_t _height);

    bool ProcessMessages();
    // only because you *really* shouldn't change these.
    [[nodiscard]] std::wstring GetTitle() const { return title; }

    [[nodiscard]] std::wstring GetClassname() const { return classname; }

    [[nodiscard]] HWND GetHwnd() const { return hwnd; }

    [[nodiscard]] Gfx &GetGfx() {
        if (!graphics) {
            // throw exception
        }

        return *graphics;
    }

    ~Window() {
        DestroyWindow(hwnd);
        UnregisterClass(classname.c_str(), hInst);
    }

private:
    std::unique_ptr<Gfx> graphics = nullptr;
    HWND hwnd;
    HINSTANCE hInst;
    size_t width, height;
    std::wstring title;
    std::wstring classname;
};


#endif // !WINDOW_UTIL_H