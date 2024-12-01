/// from https://github.com/SegaraRai/PathRedirector

#define WIN32_NO_STATUS
#include <Windows.h>
#undef WIN32_NO_STATUS

#include <iostream>
#include <MinHook.h>

#include <winternl.h>
#include <ntstatus.h>

#include <optional>
#include <string>
#include <memory>
#include <stdexcept>

#include "../common/env.h"

using namespace std::literals;

using TNtCreateFile = NTSTATUS(WINAPI*) (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER,
                                         ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
using TNtOpenFile = NTSTATUS(WINAPI*) (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, ULONG, ULONG);
using TNtSetInformationFile = NTSTATUS(WINAPI*) (HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS);

static constexpr int FileRenameInformation = 10;
static constexpr int FileRenameInformationEx = 65;

static bool gHooked = false;
static Env gEnv;

static TNtCreateFile gOrgNtCreateFile;
static TNtOpenFile gOrgNtOpenFile;
static TNtSetInformationFile gOrgNtSetInformationFile;

struct FILE_RENAME_INFORMATION {
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10_RS1)
    union {
        BOOLEAN ReplaceIfExists; // FileRenameInformation
        ULONG Flags; // FileRenameInformationEx
    } DUMMYUNIONNAME;
#else
    BOOLEAN ReplaceIfExists;
#endif
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
};

static std::optional<std::wstring> ModifyPath (const std::wstring &src) {
    // only handle \??\ and ignore other paths

    if (src.size() < 4 || src.substr(0, 4) != L"\\??\\") {
        return std::nullopt;
    }

    // remove \??\ and then apply rules
    std::wstring src_without_prefix = src.substr(4);
    std::wstring dst_without_prefix = gEnv.apply_rules(src_without_prefix);

    // add \??\ back
    std::wstring dst = L"\\??\\"s + dst_without_prefix;

#if DEBUG
    std::wstring debug_info = L"@MilkFeng ModifyPath: "s + src + L" -> "s + dst + L"\n"s;
    OutputDebugStringW(debug_info.c_str());
#endif

    return dst;
}

static NTSTATUS ModifyObjectAttributes (OBJECT_ATTRIBUTES &objectAttributes, UNICODE_STRING &unicodeString,
                                        std::wstring &unicodeStringBuffer) noexcept {
    if (!objectAttributes.ObjectName->Buffer) {
        return STATUS_ALREADY_COMPLETE;
    }

    try {
        const std::wstring src(objectAttributes.ObjectName->Buffer,
                               objectAttributes.ObjectName->Length / sizeof(wchar_t));

        const auto ret = ModifyPath(src);
        if (!ret) {
            return STATUS_ALREADY_COMPLETE;
        }

        unicodeStringBuffer = ret.value();
        unicodeString.Buffer = const_cast<PWSTR>(unicodeStringBuffer.c_str());
        unicodeString.Length = unicodeStringBuffer.size() * sizeof(wchar_t); // Length does not include null terminator
        unicodeString.MaximumLength = unicodeStringBuffer.size() * sizeof(wchar_t);
        objectAttributes.ObjectName = &unicodeString;

        return STATUS_SUCCESS;
    } catch (...) {
    }

    return STATUS_UNSUCCESSFUL;
}

template<TNtCreateFile * OrgNtZwCreateFile>
static NTSTATUS WINAPI HookedNtZwCreateFile (PHANDLE FileHandle, ACCESS_MASK DesiredAccess,
                                             POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock,
                                             PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess,
                                             ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer,
                                             ULONG EaLength) {
    try {
        do {
            if (!ObjectAttributes) {
                break;
            }
            std::wstring unicodeStringBuffer;
            UNICODE_STRING unicodeString;
            OBJECT_ATTRIBUTES NewObjectAttributes(*ObjectAttributes);
            if (const auto status = ModifyObjectAttributes(NewObjectAttributes, unicodeString, unicodeStringBuffer);
                status != STATUS_SUCCESS) {
                if (status == STATUS_ALREADY_COMPLETE) {
                    break;
                }
                return status;
            }
            return (*OrgNtZwCreateFile)(FileHandle, DesiredAccess, &NewObjectAttributes, IoStatusBlock, AllocationSize,
                                        FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer,
                                        EaLength);
        } while (false);
        return (*OrgNtZwCreateFile)(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize,
                                    FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
    } catch (...) {
        return STATUS_UNSUCCESSFUL;
    }
}


template<TNtOpenFile * OrgNtZwOpenFile>
static NTSTATUS WINAPI HookedNtZwOpenFile (PHANDLE FileHandle, ACCESS_MASK DesiredAccess,
                                           POBJECT_ATTRIBUTES ObjectAttributes,
                                           PIO_STATUS_BLOCK IoStatusBlock, ULONG ShareAccess, ULONG OpenOptions) {
    try {
        do {
            if (!ObjectAttributes) {
                break;
            }
            std::wstring unicodeStringBuffer;
            UNICODE_STRING unicodeString;
            OBJECT_ATTRIBUTES NewObjectAttributes(*ObjectAttributes);
            if (const auto status = ModifyObjectAttributes(NewObjectAttributes, unicodeString, unicodeStringBuffer);
                status != STATUS_SUCCESS) {
                if (status == STATUS_ALREADY_COMPLETE) {
                    break;
                }
                return status;
            }
            return (*OrgNtZwOpenFile)(FileHandle, DesiredAccess, &NewObjectAttributes, IoStatusBlock, ShareAccess,
                                      OpenOptions);
        } while (false);
        return (*OrgNtZwOpenFile)(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
    } catch (...) {
        return STATUS_UNSUCCESSFUL;
    }
}

template<TNtSetInformationFile * OrgNtZwSetInformationFile>
static NTSTATUS WINAPI HookedNtZwSetInformationFile (HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock,
                                                     PVOID FileInformation,
                                                     ULONG Length, FILE_INFORMATION_CLASS FileInformationClass) {
    try {
        do {
            if (FileInformationClass != FileRenameInformation && FileInformationClass != FileRenameInformationEx) {
                break;
            }
            if (!FileInformation) {
                break;
            }
            if (Length < sizeof(FILE_RENAME_INFORMATION)) {
                break;
            }

            const auto &fileRenameInformation = *static_cast<const FILE_RENAME_INFORMATION *>(FileInformation);

            if (fileRenameInformation.RootDirectory != nullptr) {
                break;
            }

            if (fileRenameInformation.FileNameLength % sizeof(wchar_t) != 0) {
                break;
            }

            if (Length < sizeof(FILE_RENAME_INFORMATION) - sizeof(FILE_RENAME_INFORMATION::FileName) +
                fileRenameInformation.FileNameLength) {
                break;
            }

            const std::wstring src(&fileRenameInformation.FileName[0],
                                   fileRenameInformation.FileNameLength / sizeof(wchar_t));
            const auto ret = ModifyPath(src);
            if (!ret) {
                break;
            }

            const auto newLength = sizeof(FILE_RENAME_INFORMATION) - sizeof(FILE_RENAME_INFORMATION::FileName) + (
                                       ret.value().size() + 1) * sizeof(wchar_t);
            auto newFileRenameInformationBuffer = std::make_unique<char[]>(newLength);
            auto ptrNewFileRenameInformationBuffer = reinterpret_cast<FILE_RENAME_INFORMATION *>(
                newFileRenameInformationBuffer.get());

            *ptrNewFileRenameInformationBuffer = fileRenameInformation;
            ptrNewFileRenameInformationBuffer->FileNameLength = ret.value().size() * sizeof(wchar_t);
            std::wmemcpy(&ptrNewFileRenameInformationBuffer->FileName[0], ret.value().c_str(),
                         (ret.value().size() + 1) * sizeof(wchar_t));

            return (*OrgNtZwSetInformationFile)(FileHandle, IoStatusBlock, ptrNewFileRenameInformationBuffer, newLength,
                                                FileInformationClass);
        } while (false);
        return (*OrgNtZwSetInformationFile)(FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass);
    } catch (...) {
        return STATUS_UNSUCCESSFUL;
    }
}

static void CheckMinHookResult (const std::string &funcName, MH_STATUS mhStatus) {
    if (mhStatus != MH_OK) {
        const std::string message = funcName + " failed with code "s + std::to_string(mhStatus);
        throw std::runtime_error(message);
    } else {
        std::cout << funcName << " success"s << std::endl;
    }
}


static void Hook () {
    gEnv = Env::read_env();

#if DEBUG
    std::wstring debug_info = L"@MilkFeng Rules[0]: "s + gEnv.rules[0].src_base + L" "s + gEnv.rules[0].src.c_str() +
                              L" "s + gEnv.rules[0].dst_name + L"\n"s;
    OutputDebugStringW(debug_info.c_str());
#endif

    CheckMinHookResult("MH_Initialize"s, MH_Initialize());

    MH_STATUS NtCreateFileHookResult = MH_CreateHookApi(
        L"ntdll.dll", "NtCreateFile",
        reinterpret_cast<LPVOID>(HookedNtZwCreateFile<&gOrgNtCreateFile>),
        reinterpret_cast<void **>(&gOrgNtCreateFile)
    );
    MH_STATUS NtOpenFileHookResult = MH_CreateHookApi(
        L"ntdll.dll", "NtOpenFile",
        reinterpret_cast<LPVOID>(HookedNtZwOpenFile<&gOrgNtOpenFile>),
        reinterpret_cast<void **>(&gOrgNtOpenFile)
    );
    MH_STATUS NtSetInformationFileHookResult = MH_CreateHookApi(
        L"ntdll.dll", "NtSetInformationFile",
        reinterpret_cast<LPVOID>(HookedNtZwSetInformationFile<&gOrgNtSetInformationFile>),
        reinterpret_cast<void **>(&gOrgNtSetInformationFile)
    );

    CheckMinHookResult("MH_CreateHookApi of NtCreateFile"s, NtCreateFileHookResult);
    CheckMinHookResult("MH_CreateHookApi of NtOpenFile"s, NtOpenFileHookResult);
    CheckMinHookResult("MH_CreateHookApi of NtSetInformationFile"s, NtSetInformationFileHookResult);

    CheckMinHookResult("MH_EnableHook"s, MH_EnableHook(MH_ALL_HOOKS));
}


static void Unhook () {
    CheckMinHookResult("MH_DisableHook"s, MH_DisableHook(MH_ALL_HOOKS));
    CheckMinHookResult("MH_Uninitialize"s, MH_Uninitialize());
}


BOOL WINAPI DllMain (HINSTANCE hInstance, ULONG ulReason, LPVOID Reserved) {
#if DEBUG
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
