﻿#pragma once
#include <regex>

/// Removes directories that don't exist, and tries to make their paths relative if they are contained within the current directory.
inline void processDirectoryPaths(std::vector<std::string>& directoryPaths, const bool reverse)
{
    std::vector<std::string> newDirectoryPaths;

    const std::filesystem::path currentPath = std::filesystem::current_path().lexically_normal();

    for (auto& directoryPath : directoryPaths)
    {
        const std::filesystem::path path = std::filesystem::path(directoryPath).lexically_normal();

        if (!std::filesystem::is_directory(path))
            continue;

        const std::string newDirectoryPath = path.string();
        const std::string relativePath = std::filesystem::relative(path, currentPath).string();

        std::string newPath = !relativePath.empty() && relativePath.size() < newDirectoryPath.size() ? relativePath : newDirectoryPath;

        std::replace(newPath.begin(), newPath.end(), '\\', '/');

        if (!newPath.empty() && newPath.back() == '/')
            newPath.pop_back();

        newDirectoryPaths.push_back(newPath);
    }

    if (reverse)
        std::reverse(newDirectoryPaths.begin(), newDirectoryPaths.end());

    std::swap(directoryPaths, newDirectoryPaths);
}

/// Removes files that don't exist, and tries to make their paths relative if they are contained within the current directory.
inline void processFilePaths(std::vector<std::wstring>& filePaths, const bool reverse)
{
    std::vector<std::wstring> newFilePaths;

    for (auto& filePath : filePaths)
    {
        if (!std::filesystem::is_regular_file(filePath))
            continue;

        std::wstring newPath = std::filesystem::absolute(filePath).lexically_normal().wstring();
        std::replace(newPath.begin(), newPath.end(), L'\\', L'/');

        newFilePaths.push_back(newPath);
    }

    if (reverse)
        std::reverse(newFilePaths.begin(), newFilePaths.end());

    std::swap(filePaths, newFilePaths);
}

/// Formats file path for logging.
inline const wchar_t* processFilePath(const wchar_t* filePath)
{
    std::wstring newPath = std::regex_replace(filePath, std::wregex(L"\\./"), L"/");
    newPath = std::regex_replace(newPath, std::wregex(L"(/+)"), L"/");
    return newPath.c_str();
}

inline std::wstring convertMultiByteToWideChar(const std::string& value)
{
    WCHAR wideChar[0x400];
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, wideChar, _countof(wideChar));

    return std::wstring(wideChar);
}

/// Gets relative path if it's contained within the current directory.
inline std::string getRelativePath(const std::string& filePath)
{
    std::string relativePath = std::filesystem::relative(filePath, std::filesystem::current_path()).string();

    if (relativePath.empty() || relativePath.size() > filePath.size())
        relativePath = filePath;

    std::replace(relativePath.begin(), relativePath.end(), '\\', '/');

    return relativePath;
}

/// Gets relative path if it's contained within the current directory.
inline std::wstring getRelativePath(const std::wstring& filePath)
{
    std::wstring relativePath = std::filesystem::relative(filePath, std::filesystem::current_path()).wstring();

    if (relativePath.empty() || relativePath.size() > filePath.size())
        relativePath = filePath;

    std::replace(relativePath.begin(), relativePath.end(), L'\\', L'/');

    return relativePath;
}

inline uint32_t readUnalignedU32(void* memory)
{
    uint8_t* p = (uint8_t*)memory;
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}