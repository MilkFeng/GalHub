#ifndef NT_OPEN_FILE_HPP
#define NT_OPEN_FILE_HPP

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <ntstatus.h>

#include <string>

#include "path_util.h"

using TNtOpenFile = NTSTATUS(WINAPI*) (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, ULONG, ULONG);


template<TNtOpenFile * OrgNtZwOpenFile>
static NTSTATUS WINAPI HookedNtOpenFile (PHANDLE FileHandle, ACCESS_MASK DesiredAccess,
                                         POBJECT_ATTRIBUTES ObjectAttributes,
                                         PIO_STATUS_BLOCK IoStatusBlock, ULONG ShareAccess, ULONG OpenOptions) {
#ifdef DEBUG
    std::wstring debug_info = L"@MilkFeng HookedNtZwOpenFile\n";
    OutputDebugStringW(debug_info.c_str());
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
            return (*OrgNtZwOpenFile)(FileHandle, DesiredAccess, &NewObjectAttributes, IoStatusBlock, ShareAccess,
                                      OpenOptions);
        } while (false);
        return (*OrgNtZwOpenFile)(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
    } catch (...) {
        return STATUS_UNSUCCESSFUL;
    }
}

#endif //NT_OPEN_FILE_HPP
