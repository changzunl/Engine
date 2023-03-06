//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include "Engine/Window/Dialog.hpp"

#include <windows.h>      // For common windows data types and function headers

std::wstring GetCurrentDirW()
{
    std::wstring dir;
    dir.resize(255);

    do
    {
        auto read = ::GetCurrentDirectoryW((uint32_t)dir.size(), dir.data());

        if (read <= dir.size())
        {
            dir.resize(read);
            break;
        }

        dir.resize(read);
    } while (true);

    return dir;
}

std::string GetCurrentDir()
{
    std::string dir;
    dir.resize(255);

    do
    {
        auto read = ::GetCurrentDirectoryA((uint32_t)dir.size(), dir.data());

        if (read <= dir.size())
        {
            dir.resize(read);
            break;
        }

        dir.resize(read);
    } while (true);

    return dir;
}

std::wstring BasicFileOpenW(void* hWnd, const wchar_t* filter)
{
    OPENFILENAME ofn = {};       // common dialog box structure
    TCHAR szFile[260] = { 0 };       // if using TCHAR macros

    std::wstring dir = GetCurrentDirW();

    // Initialize OPENFILENAME
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = HWND(hWnd);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = dir.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn) == TRUE)
    {
        return ofn.lpstrFile;
    }

    return {};
}

std::string BasicFileOpen(void* hWnd, const char* filter)
{
    OPENFILENAMEA ofn = {};       // common dialog box structure
    CHAR szFile[260] = { 0 };       // if using TCHAR macros

    std::string dir = GetCurrentDir();

    // Initialize OPENFILENAME
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = HWND(hWnd);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = dir.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        return ofn.lpstrFile;
    }

    return "";
}

std::wstring BasicFileSaveW(void* hWnd, const wchar_t* filter, const wchar_t* ext)
{
    OPENFILENAME ofn = {};       // common dialog box structure
    TCHAR szFile[260] = { 0 };       // if using TCHAR macros

    std::wstring dir = GetCurrentDirW();

    // Initialize OPENFILENAME
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = HWND(hWnd);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = ext;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = dir.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetSaveFileNameW(&ofn) == TRUE)
    {
        return ofn.lpstrFile;
    }

    return {};
}

std::string BasicFileSave(void* hWnd, const char* filter, const char* ext)
{
    OPENFILENAMEA ofn = {};       // common dialog box structure
    CHAR szFile[260] = { 0 };       // if using TCHAR macros

    std::string dir = GetCurrentDir();

    // Initialize OPENFILENAME
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = HWND(hWnd);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = ext;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = dir.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetSaveFileNameA(&ofn) == TRUE)
    {
        return ofn.lpstrFile;
    }

    return "";
}
