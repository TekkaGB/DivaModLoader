#include "FileLogger.h"
#include "Context.h"
#include "Utilities.h"
#include <regex>

HOOK(HRESULT, __stdcall, CreateFileW, &CreateFileW,
    LPCWSTR               lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile
)
{
    if (std::regex_match(lpFileName, std::wregex(L"^(\\./rom_steam/)(.*)")))
        LOG("[File Logger] Accessed file: %S", processFilePath(lpFileName))
    return originalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

void FileLogger::init()
{
    LOG("Initializing File Logger...")
    INSTALL_HOOK(CreateFileW)
}