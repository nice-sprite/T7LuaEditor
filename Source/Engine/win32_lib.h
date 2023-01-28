#ifndef WINDOW_UTIL_H
#define WINDOW_UTIL_H

#include <Windows.h>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <functional>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <wrl/client.h>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace win32 {
struct Window {
  HWND hwnd;
  RECT client_rect;
};

Window create_window(HINSTANCE hinst,
                     const char *windowTitle,
                     const char *classname,
                     int width,
                     int height,
                     WNDPROC proc,
                     const char *window_icon_path,
                     void* userdata);

void set_window_icon(HWND hwnd, const char *iconPath);

using DxgiAdapterList = std::vector<DXGI_ADAPTER_DESC1>;

DxgiAdapterList get_gpu_specs();

std::vector<std::string>
dxgi_adapter_list_to_strings(DxgiAdapterList const &adapters);

typedef BOOL(WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

std::string get_cpu_specs();

double bytes_to_gigabytes(SIZE_T bytes);

// Helper function to count set bits in the processor mask.
DWORD count_set_bits(ULONG_PTR bitMask);

}; // namespace win32

#endif // !WINDOW_UTIL_H
