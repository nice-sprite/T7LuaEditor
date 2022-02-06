//
// Created by coxtr on 11/29/2021.
//

#ifndef T7LUAEDITOR_SYSTEMINFO_H
#define T7LUAEDITOR_SYSTEMINFO_H

#include <wrl/client.h>
#include <dxgi.h>
#include <vector>
#include <fmt/format.h>

namespace wrl = Microsoft::WRL;
using DxgiAdapterList = std::vector<DXGI_ADAPTER_DESC1>;

DxgiAdapterList GetGPUInfo();

typedef BOOL (WINAPI* LPFN_GLPI)(
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
        PDWORD);

std::string GetCPUInfo();

double BytesToGigabytes(SIZE_T bytes);

// Helper function to count set bits in the processor mask.
DWORD CountSetBits(ULONG_PTR bitMask);

std::vector<std::string> ToString(DxgiAdapterList adapters);

#endif //T7LUAEDITOR_SYSTEMINFO_H
