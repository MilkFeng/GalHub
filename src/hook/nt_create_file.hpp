#ifndef NT_CREATE_FILE_HPP
#define NT_CREATE_FILE_HPP

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <ntstatus.h>

#include <string>
#include <memory>

#include "path_util.h"

using TNtCreateFile = NTSTATUS(WINAPI*) (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER,
                                         ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);


template<TNtCreateFile * OrgNtZwCreateFile>
NTSTATUS WINAPI HookedNtCreateFile (PHANDLE FileHandle, ACCESS_MASK DesiredAccess,
                                    POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock,
                                    PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess,
                                    ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer,
                                    ULONG EaLength) {
#ifdef DEBUG
    {
        std::wstring DesiredAccess_str = std::to_wstring(DesiredAccess);
        std::wstring ObjectAttributes_str = L"{" + std::to_wstring(ObjectAttributes->Length) + L", "s +
                                            std::to_wstring(ObjectAttributes->RootDirectory == nullptr) + L", "s +
                                            std::to_wstring(ObjectAttributes->ObjectName->Length) + L", "s +
                                            std::to_wstring(ObjectAttributes->ObjectName->MaximumLength) + L", "s +
                                            std::to_wstring(ObjectAttributes->ObjectName->Buffer == nullptr) + L"}"s;
        std::wstring AllocationSize_str = std::to_wstring(AllocationSize == nullptr);
        std::wstring ShareAccess_str = std::to_wstring(ShareAccess);
        std::wstring CreateDisposition_str = std::to_wstring(CreateDisposition);
        std::wstring CreateOptions_str = std::to_wstring(CreateOptions);
        std::wstring EaBuffer_str = std::to_wstring(EaBuffer == nullptr);
        std::wstring EaLength_str = std::to_wstring(EaLength);
        std::wstring debug_info = L"@MilkFeng HookedNtZwCreateFile: " + DesiredAccess_str + L"; " + ObjectAttributes_str +
                                  L"; " + AllocationSize_str + L"; " + ShareAccess_str + L"; " + CreateDisposition_str +
                                  L"; " + CreateOptions_str + L"; " + EaBuffer_str + L"; " + EaLength_str + L"\n";

        OutputDebugStringW(debug_info.c_str());
    }
#endif

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


#endif //NT_CREATE_FILE_HPP
