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

// Implementations of the API.
bool AuthorName(va_list Variadic)
{
    char *Result = va_arg(Variadic, char *);
    strcpy_s(Result, 512, "Convery");
    return true;
}
bool AuthorSite(va_list Variadic)
{
    char *Result = va_arg(Variadic, char *);
    strcpy_s(Result, 512, "https://xn--wxa.ayria.io/LocalNetworking");
    return true;
}
bool Internalname(va_list Variadic)
{
    char *Result = va_arg(Variadic, char *);
    strcpy_s(Result, 512, "LocalNetworking - Like OpenNet");
    return true;
}
bool Modulehash(va_list Variadic)
{
    size_t *Result = va_arg(Variadic, size_t *);

    // Replace this value after compiling.
    *Result = 123456789;

    return true;
}
bool DependencyCount(va_list Variadic)
{
    size_t *Result = va_arg(Variadic, size_t *);

    // No dependencies for this plugin.
    *Result = 0;

    return true;
}
bool DependencyName(va_list Variadic)
{
    size_t Index = va_arg(Variadic, size_t);
    char *Result = va_arg(Variadic, char *);

    switch (Index)
    {
    default:
        strcpy_s(Result, 512, "404");
    }

    return true;
}
bool WhitelistClaim(va_list Variadic)
{
    bool *Result = va_arg(Variadic, bool *);
    *Result = false;

    return true;
}
bool ReadValue(va_list Variadic)
{
    size_t Key = va_arg(Variadic, size_t);
    char *Value = va_arg(Variadic, char *);

    // We don't save data for other plugins.
    return true;
}
bool WriteValue(va_list Variadic)
{
    size_t Key = va_arg(Variadic, size_t);
    char *Value = va_arg(Variadic, char *);

    // We don't save data for other plugins.
    return true;
}
bool HandleMessage(va_list Variadic)
{
    char *Pluginname = va_arg(Variadic, char *);
    size_t Command = va_arg(Variadic, size_t);
    char *Message = va_arg(Variadic, char *);

    switch (Command)
    {

    default:
        DebugPrint(va("Got a message from plugin \"%s\" with value: \"%s\"", Pluginname, Message));
    }

    return true;
}
bool PreInitialization(va_list Variadic)
{
    bool *Result = va_arg(Variadic, bool *);

    // Platform dependant logic.
#ifdef _WIN32
    if (!NTServerManager::InitializeWinsockHooks()) return false;
#else
    if (!NIXServerManager::InitializeWinsockHooks()) return false;
#endif

    // Load all the servers we'll be using.
    *Result = ModuleLoader::LoadFromCSV("AyriaModules");

    return true;
}
bool PostInitialization(va_list Variadic)
{
    bool *Result = va_arg(Variadic, bool *);
    *Result = true;

    return true;
}
bool NotifyShutdown(va_list Variadic)
{
    // Platform dependant logic.
#ifdef _WIN32 
    NTServerManager::SendShutdown();                
#else
    NIXServerManager::SendShutdown();
#endif

    return true;
}

#define EXPORTFUNCTION(Command, Functor)    \
    case FNV1a_Compiletime(Command):        \
        Result = Functor(Variadic);         \
        break

extern "C"
{
    // Ayrias plugin exports as per https://github.com/AyriaNP/Documentation/blob/master/Plugins/StandardExports.md
    EXPORT_ATTR bool __cdecl AyriaPlugin(size_t Command, ...)
    {
        bool Result = false;
        va_list Variadic;
        va_start(Variadic, Command);

        switch (Command)
        {
            EXPORTFUNCTION("AuthorName", AuthorName);
            EXPORTFUNCTION("AuthorSite", AuthorSite);
            EXPORTFUNCTION("Internalname", Internalname);
            EXPORTFUNCTION("Modulehash", Modulehash);
            EXPORTFUNCTION("DependencyCount", DependencyCount);
            EXPORTFUNCTION("DependencyName", DependencyName);
            EXPORTFUNCTION("WhitelistClaim", WhitelistClaim);
            EXPORTFUNCTION("ReadValue", ReadValue);
            EXPORTFUNCTION("WriteValue", WriteValue);
            EXPORTFUNCTION("HandleMessage", HandleMessage);
            EXPORTFUNCTION("PreInitialization", PreInitialization);
            EXPORTFUNCTION("PostInitialization", PostInitialization);
            EXPORTFUNCTION("NotifyShutdown", NotifyShutdown);
        }

        va_end(Variadic);
        return Result;
    }
}

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
        AyriaPlugin(FNV1a_Compiletime("NotifyShutdown"));
        break;
    }

    return TRUE;
}
