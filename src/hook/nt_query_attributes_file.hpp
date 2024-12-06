#ifndef NT_QUERY_ATTRIBUTES_FILE_HPP
#define NT_QUERY_ATTRIBUTES_FILE_HPP


#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <ntstatus.h>

#include <string>

#include "path_util.h"


typedef struct _FILE_BASIC_INFORMATION {
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;


using TNtQueryAttributesFile = NTSTATUS(WINAPI*) (POBJECT_ATTRIBUTES, PFILE_BASIC_INFORMATION);


template<TNtQueryAttributesFile * OrgNtQueryAttributesFile>
NTSTATUS WINAPI HookedNtQueryAttributesFile (POBJECT_ATTRIBUTES ObjectAttributes, PFILE_BASIC_INFORMATION FileInformation) {
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
            return (*OrgNtQueryAttributesFile)(&NewObjectAttributes, FileInformation);
        } while (false);
        return (*OrgNtQueryAttributesFile)(ObjectAttributes, FileInformation);
    } catch (...) {
        return STATUS_UNSUCCESSFUL;
    }

}


#endif //NT_QUERY_ATTRIBUTES_FILE_HPP
