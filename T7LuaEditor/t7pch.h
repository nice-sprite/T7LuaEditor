//
// Created by coxtr on 12/27/2021.
//

#ifndef T7LUAEDITOR_T7PCH_H
#define T7LUAEDITOR_T7PCH_H

#include "Engine/Timer.h"
#include <algorithm>
#include <cassert>
#include <d2d1_3.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dwrite_3.h>
#include <dxgi1_6.h>
#include <entt/entt.hpp>
#include <fmt/core.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <memory>
#include <simdjson.h>
#include <string>
#include <vector>
#include <wrl/client.h>
#include <thread>
#include <filesystem>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler
#pragma comment( lib, "d2d1.lib")
#pragma comment( lib, "dwrite.lib")
#pragma comment( lib, "dxguid.lib")

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
using namespace DirectX;

#endif //T7LUAEDITOR_T7PCH_H
