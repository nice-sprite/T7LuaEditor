//
// Created by coxtr on 11/29/2021.
//

#include "SystemDeviceInfoManager.h"

SystemDeviceInfoManager::SystemDeviceInfoManager() {
    CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **) dxgi.GetAddressOf());
    IDXGIAdapter1 *currentAdapter;
    for (UINT i = 0; dxgi->EnumAdapters1(i, &currentAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        wrl::ComPtr<IDXGIAdapter1> a = currentAdapter;
        availableAdapters.push_back(a);
        DXGI_ADAPTER_DESC1 desc{};
        a->GetDesc1(&desc);
        adapterDefs.push_back(desc);
    }
}

DXGI_OUTPUT_DESC SystemDeviceInfoManager::GetInfo() {
    return DXGI_OUTPUT_DESC();
}

std::vector<std::string> SystemDeviceInfoManager::ToString() {
    std::vector<std::string> defStrings{};

    auto formatStr = R"(Description: {}
VendorId: {}
DeviceId: {}
SubSysId: {}
Revision: {}
DedicatedVideoMem: {:.2f}
DedicatedSystemMem: {:.2f}
SharedSystemMem: {:.2f}
AdapterLuid: {}
Flags: {})";

    for (auto &def: adapterDefs) {
        size_t numConverted;
        char cstrDescription[128]{};
        wcstombs_s(&numConverted, cstrDescription, 128, def.Description, _TRUNCATE);
        auto strRep = fmt::format(formatStr, cstrDescription, def.VendorId, def.DeviceId, def.SubSysId, def.Revision,
                                  BytesToGigabytes(def.DedicatedVideoMemory),
                                  BytesToGigabytes(def.DedicatedSystemMemory), BytesToGigabytes(def.SharedSystemMemory), def.AdapterLuid.LowPart, def.Flags);

        defStrings.push_back(strRep);

    }
    return defStrings;

}
