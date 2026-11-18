#pragma once
#ifdef _WIN32
#include <windows.h>
inline void enableUtf8Console()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}
#else
inline void enableUtf8Console() {}
#endif