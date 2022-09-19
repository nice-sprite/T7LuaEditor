#ifndef WINDOW_UTIL_H
#define WINDOW_UTIL_H

#include "./win32_input.h"
#include <functional>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <Windows.h>
#include <imgui_impl_win32.h>
#include <imgui.h>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace win32 
{
    struct Window
    {
        HWND hwnd;
        RECT client_rect;
    };

    Window create_window(
        HINSTANCE hinst,
        const wchar_t *windowTitle,
        const wchar_t *classname,
        int width,
        int height,
        WNDPROC proc,
        const wchar_t* window_icon_path
    );

    void set_window_icon(HWND hwnd, const wchar_t *iconPath);

    struct Timer
    {
        __int64 frequency;
        __int64 a;
        __int64 b;
        __int64 paused;
        double seconds_per_count;
        double elapsed;

        Timer();

        double elapsed_ms();
        void tick();
        void start();
        void stop();
        void reset();
    };

    using DxgiAdapterList = std::vector<DXGI_ADAPTER_DESC1>;

    DxgiAdapterList get_gpu_specs();
    std::vector<std::string> dxgi_adapter_list_to_strings(DxgiAdapterList const& adapters);

    typedef BOOL (WINAPI* LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

    std::string get_cpu_specs();

    double bytes_to_gigabytes(SIZE_T bytes);

    // Helper function to count set bits in the processor mask.
    DWORD count_set_bits(ULONG_PTR bitMask);

};

#endif // !WINDOW_UTIL_H
