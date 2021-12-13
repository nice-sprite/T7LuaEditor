//
// Created by coxtr on 11/29/2021.
//

#include "SystemInfo.h"


DxgiAdapterList GetGPUInfo() {
    wrl::ComPtr<IDXGIFactory1> dxgi;
    std::vector<wrl::ComPtr<IDXGIAdapter1>> availableAdapters;
    std::vector<DXGI_ADAPTER_DESC1> adapterDefs;
    CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **) dxgi.GetAddressOf());
    IDXGIAdapter1 *currentAdapter;
    for (UINT i = 0; dxgi->EnumAdapters1(i, &currentAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        wrl::ComPtr<IDXGIAdapter1> a = currentAdapter;
        availableAdapters.push_back(a);
        DXGI_ADAPTER_DESC1 desc{};
        a->GetDesc1(&desc);
        adapterDefs.push_back(desc);
    }
    return adapterDefs;
}

std::string GetCPUInfo() {
    LPFN_GLPI glpi;
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = nullptr;
    DWORD returnLength = 0;
    DWORD logicalProcessorCount = 0;
    DWORD numaNodeCount = 0;
    DWORD processorCoreCount = 0;
    DWORD processorL1CacheCount = 0;
    DWORD processorL2CacheCount = 0;
    DWORD processorL3CacheCount = 0;
    DWORD processorPackageCount = 0;
    DWORD byteOffset = 0;
    PCACHE_DESCRIPTOR Cache;

    glpi = (LPFN_GLPI) GetProcAddress(
            GetModuleHandle(TEXT("kernel32")),
            "GetLogicalProcessorInformation");
    if (nullptr == glpi) {
        return "\nGetLogicalProcessorInformation is not supported.\n";
    }

    while (!done) {
        DWORD rc = glpi(buffer, &returnLength);

        if (FALSE == rc) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                if (buffer)
                    free(buffer);

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION) malloc(
                        returnLength);

                if (nullptr == buffer) {
                    return "\nError: Allocation failure\n";

                }
            } else {
                return "\nError \n";
            }
        } else {
            done = TRUE;
        }
    }

    ptr = buffer;

    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) {
        switch (ptr->Relationship) {
            case RelationNumaNode:
                // Non-NUMA systems report a single record of this type.
                numaNodeCount++;
                break;

            case RelationProcessorCore:
                processorCoreCount++;

                // A hyperthreaded core supplies more than one logical processor.
                logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
                break;

            case RelationCache:
                // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache.
                Cache = &ptr->Cache;
                if (Cache->Level == 1) {
                    processorL1CacheCount++;
                } else if (Cache->Level == 2) {
                    processorL2CacheCount++;
                } else if (Cache->Level == 3) {
                    processorL3CacheCount++;
                }
                break;

            case RelationProcessorPackage:
                // Logical processors share a physical package.
                processorPackageCount++;
                break;

            default:
                (TEXT("\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n"));
                break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

/*
    //(TEXT("\nGetLogicalProcessorInformation results:\n"));
    //(TEXT("Number of NUMA nodes: %d\n"),
           numaNodeCount);
    //(TEXT("Number of physical processor packages: %d\n"),
           processorPackageCount);
    //(TEXT("Number of processor cores: %d\n"),
           processorCoreCount);
    //(TEXT("Number of logical processors: %d\n"),
           logicalProcessorCount);
    //(TEXT("Number of processor L1/L2/L3 caches: %d/%d/%d\n"),
           processorL1CacheCount,
           processorL2CacheCount,
           processorL3CacheCount);
*/

    auto sysCPUInfo = fmt::format("NUMA nodes: {}\n"
                                  "physical processors: {}\n"
                                  "processor cores: {}\n"
                                  "logical processors: {}\n"
                                  "L1/L2/L3 caches: {}{}{}\n"
                                  "Cache line size: {}", numaNodeCount, processorPackageCount,
                                  processorCoreCount, logicalProcessorCount,
                                  processorL1CacheCount,
                                  processorL2CacheCount, processorL3CacheCount, buffer->Cache.LineSize);

    free(buffer);

    return sysCPUInfo;
}

double BytesToGigabytes(SIZE_T bytes) {
    return (double) bytes / (double) (1 << 30);
}

DWORD CountSetBits(ULONG_PTR bitMask) {
    DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR) 1 << LSHIFT;
    DWORD i;

    for (i = 0; i <= LSHIFT; ++i) {
        bitSetCount += ((bitMask & bitTest) ? 1 : 0);
        bitTest /= 2;
    }

    return bitSetCount;
}

std::vector<std::string> ToString(DxgiAdapterList adapters) {
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

    for (auto &def: adapters) {
        size_t numConverted;
        char cstrDescription[128]{};
        wcstombs_s(&numConverted, cstrDescription, 128, def.Description, _TRUNCATE);
        auto strRep = fmt::format(formatStr, cstrDescription, def.VendorId, def.DeviceId, def.SubSysId, def.Revision,
                                  BytesToGigabytes(def.DedicatedVideoMemory),
                                  BytesToGigabytes(def.DedicatedSystemMemory), BytesToGigabytes(def.SharedSystemMemory),
                                  def.AdapterLuid.LowPart, def.Flags);

        defStrings.push_back(strRep);

    }
    return defStrings;

}
