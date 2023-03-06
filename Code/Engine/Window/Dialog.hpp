#pragma once

#include <string>

typedef long HRESULT;

std::wstring BasicFileOpenW(void* hWnd, const wchar_t* filter = nullptr);
std::string BasicFileOpen(void* hWnd, const char* filter = nullptr);

std::wstring BasicFileSaveW(void* hWnd, const wchar_t* filter = nullptr, const wchar_t* ext = nullptr);
std::string BasicFileSave(void* hWnd, const char* filter = nullptr, const char* ext = nullptr);
