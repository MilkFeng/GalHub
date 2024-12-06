/// from https://github.com/SegaraRai/PathRedirector

#include "nt.hpp"

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <iostream>
#include <MinHook.h>

#include <winternl.h>

#include <optional>
#include <string>
#include <stdexcept>

#include "path_util.h"
#include "../common/env.h"

using namespace std::literals;

static bool gHooked = false;

static TNtCreateFile gOrgNtCreateFile;
static TNtOpenFile gOrgNtOpenFile;
static TNtSetInformationFile gOrgNtSetInformationFile;
static TNtQueryAttributesFile gOrgNtQueryAttributesFile;

static void CheckMinHookResult (const std::string &funcName, MH_STATUS mhStatus) {
    if (mhStatus != MH_OK) {
        const std::string message = funcName + " failed with code "s + std::to_string(mhStatus);
        throw std::runtime_error(message);
    } else {
        std::cout << funcName << " success"s << std::endl;
    }
}


static void Hook () {
    GlobalEnv() = Env::read_env();

#ifdef DEBUG
    std::wstring debug_info = L"@MilkFeng Rules Count: "s + std::to_wstring(GlobalEnv().rules.size()) + L"\n"s;
    OutputDebugStringW(debug_info.c_str());

    if (!GlobalEnv().rules.empty()) {
        std::wstring debug_info = L"@MilkFeng Rules[0]: "s + GlobalEnv().rules[0].src_base + L"/"s + GlobalEnv().rules[0].src.c_str() + L" -> "s + GlobalEnv().rules[0].dst_name + L"\n"s;
        OutputDebugStringW(debug_info.c_str());
    }
#endif

    CheckMinHookResult("MH_Initialize"s, MH_Initialize());

    MH_STATUS NtCreateFileHookResult = MH_CreateHookApi(
        L"ntdll.dll", "NtCreateFile",
        reinterpret_cast<LPVOID>(HookedNtCreateFile<&gOrgNtCreateFile>),
        reinterpret_cast<void **>(&gOrgNtCreateFile)
    );
    MH_STATUS NtOpenFileHookResult = MH_CreateHookApi(
        L"ntdll.dll", "NtOpenFile",
        reinterpret_cast<LPVOID>(HookedNtOpenFile<&gOrgNtOpenFile>),
        reinterpret_cast<void **>(&gOrgNtOpenFile)
    );
    MH_STATUS NtQueryAttributesFileHookResult = MH_CreateHookApi(
        L"ntdll.dll", "NtQueryAttributesFile",
        reinterpret_cast<LPVOID>(HookedNtQueryAttributesFile<&gOrgNtQueryAttributesFile>),
        reinterpret_cast<void **>(&gOrgNtQueryAttributesFile)
    );
    MH_STATUS NtSetInformationFileHookResult = MH_CreateHookApi(
        L"ntdll.dll", "NtSetInformationFile",
        reinterpret_cast<LPVOID>(HookedNtSetInformationFile<&gOrgNtSetInformationFile>),
        reinterpret_cast<void **>(&gOrgNtSetInformationFile)
    );

    CheckMinHookResult("MH_CreateHookApi of NtCreateFile"s, NtCreateFileHookResult);
    CheckMinHookResult("MH_CreateHookApi of NtOpenFile"s, NtOpenFileHookResult);
    CheckMinHookResult("MH_CreateHookApi of NtQueryAttributesFile"s, NtQueryAttributesFileHookResult);
    CheckMinHookResult("MH_CreateHookApi of NtSetInformationFile"s, NtSetInformationFileHookResult);

    CheckMinHookResult("MH_EnableHook"s, MH_EnableHook(MH_ALL_HOOKS));
}


static void Unhook () {
    CheckMinHookResult("MH_DisableHook"s, MH_DisableHook(MH_ALL_HOOKS));
    CheckMinHookResult("MH_Uninitialize"s, MH_Uninitialize());
}


BOOL WINAPI DllMain (HINSTANCE hInstance, ULONG ulReason, LPVOID Reserved) {
#ifdef DEBUG
    std::wstring debug_info = L"@MilkFeng DllMain";
    OutputDebugStringW(debug_info.c_str());
#endif

    switch (ulReason) {
        case DLL_PROCESS_ATTACH:
            try {
                Hook();
                gHooked = true;
                return TRUE;
            } catch (std::exception &exception) {
                const std::string message = "Failed to Hook: "s + exception.what();
                MessageBoxA(nullptr, message.c_str(), "PathRedirector", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
            } catch (...) {
                MessageBoxW(nullptr, L"Failed to Hook", L"PathRedirector", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
            }

            return 0;

        case DLL_PROCESS_DETACH:
            try {
                if (gHooked) {
                    Unhook();
                    gHooked = false;
                }
            } catch (...) {
            }
            break;
    }
    return 1;
}
