#pragma once

// https://docs.microsoft.com/en-us/windows/desktop/winprog/using-the-windows-headers

// #include <sdkddkver.h>

// Targeting Windows 7
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <debugapi.h>
