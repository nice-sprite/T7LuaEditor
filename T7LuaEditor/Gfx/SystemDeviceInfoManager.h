//
// Created by coxtr on 11/29/2021.
//

#ifndef T7LUAEDITOR_SYSTEMDEVICEINFOMANAGER_H
#define T7LUAEDITOR_SYSTEMDEVICEINFOMANAGER_H

#include <wrl/client.h>
#include <dxgi.h>
#include <vector>
#include <fmt/format.h>
namespace wrl = Microsoft::WRL;

class SystemDeviceInfoManager {
public:
    SystemDeviceInfoManager();
    DXGI_OUTPUT_DESC GetInfo();

    // returns a string represenation of an adapter
    std::vector<std::string> ToString();
private:
    wrl::ComPtr<IDXGIFactory1> dxgi;
    std::vector<wrl::ComPtr<IDXGIAdapter1>> availableAdapters;
    std::vector<DXGI_ADAPTER_DESC1> adapterDefs;

    static double BytesToGigabytes(SIZE_T bytes) {
        return (double)bytes/(double)(1<<30);
    }
};


#endif //T7LUAEDITOR_SYSTEMDEVICEINFOMANAGER_H
