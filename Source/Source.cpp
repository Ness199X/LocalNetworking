/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-10
    Notes:
        Exported methods for initialization.
*/

#include <stdarg.h>
#include "Macros.h"
#include "Modules\ModuleLoader.h"
#include "Platform\NTServerManager.h"
#include "Utility\Crypto\FNV1.h"

// Platform specific keywords.
#ifdef __linux__
#define EXPORT_ATTR __attribute__((visibility("default")))
#define IMPORT_ATTR
#elif _WIN32
#define EXPORT_ATTR __declspec(dllexport)
#define IMPORT_ATTR __declspec(dllimport)
#else
#define EXPORT_ATTR
#define IMPORT_ATTR
#pragma warning Unknown dynamic link import/export semantics.
#endif

#define EXPORT_UNHANDLED 0
#define SERVER_NOEXPORTS 93

extern "C"
{
    // When loading this as a plugin in the ayria system.
    EXPORT_ATTR bool __cdecl AyriaPlugin(size_t Command, ...)
    {
        bool Result = false;
        va_list Variadic;
        va_start(Variadic, Command);

        switch (Command)
        {
            case FNV1a_Compiletime("PreInitialization"):
            {
                // Platform dependant logic.
#ifdef _WIN32
                if (!NTServerManager::InitializeWinsockHooks())
                {
                    SetLastError(SERVER_NOEXPORTS);
                    return false;
                }
#else
                if (!NIXServerManager::InitializeWinsockHooks())
                {
                    SetLastError(SERVER_NOEXPORTS);
                    return false;
                }
#endif

                // Load all the servers we'll be using.
                Result = ModuleLoader::LoadFromCSV("AyriaModules");
            } break;         

            case FNV1a_Compiletime("Shutdown"):
            {
                // Platform dependant logic.
#ifdef _WIN32 
                NTServerManager::SendShutdown();                
#else
                NIXServerManager::SendShutdown();
#endif

                Result = true;
            }break;

            default:
                SetLastError(EXPORT_UNHANDLED);
        }

        va_end(Variadic);
        return Result;
    }
};

BOOLEAN WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved)
{
    switch ( nReason )
    {
    case DLL_PROCESS_ATTACH:
        // Rather not handle all thread updates.
        DisableThreadLibraryCalls( hDllHandle );

        // Clean the logfile so we only save this session.
        DeleteLogfile();
        break;

    case DLL_PROCESS_DETACH:
        AyriaPlugin(FNV1a_Compiletime("Shutdown"));
        break;
    }

    return TRUE;
}
