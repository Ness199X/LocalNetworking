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

// Macros to make it more readable.
#define EXPORTSTART(ExportName) \
EXPORT_ATTR bool __cdecl ExportName(size_t Command, ...)    \
{                               \
    bool Result = false;        \
    va_list Variadic;           \
    va_start(Variadic, Command);\
                                \
    switch(Command)             \
    {
#define EXPORTEND               \
    }                           \
                                \
    va_end(Variadic);           \
    return Result;              \
}
#define EXPORTFUNCTION(Command, Functor)    \
    case FNV1a_Compiletime(Command):        \
        Result = Functor(Variadic);         \
        break

#define EXPORT_UNHANDLED 0
#define SERVER_NOEXPORTS 93

// Implementation of the exports.
bool PreInitialization(va_list Variadic)
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
    return ModuleLoader::LoadFromCSV("AyriaModules");
}
bool Shutdown(va_list Variadic)
{
    // Platform dependant logic.
#ifdef _WIN32 
    NTServerManager::SendShutdown();                
#else
    NIXServerManager::SendShutdown();
#endif

    return true;
}
bool AuthorName(va_list Variadic)
{
    char *Result = va_arg(Variadic, char *);

    // Copy a hardcoded string to the buffer.
    strcpy_s(Result, 1024, "Convery");

    return true;
}
bool AuthorSite(va_list Variadic)
{
    char *Result = va_arg(Variadic, char *);

    // Copy a hardcoded string to the buffer.
    strcpy_s(Result, 1024, "https://github.com/Convery");

    return true;
}
bool InternalName(va_list Variadic)
{
    char *Result = va_arg(Variadic, char *);

    // Copy a hardcoded string to the buffer.
    strcpy_s(Result, 1024, "LocalNetworking (like Opennet)");

    return true;
}
bool OnDiskHash(va_list Variadic)
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
        // Copy a hardcoded string to the buffer.
        strcpy_s(Result, 1024, "Invalid index");
    }

    return true;
}
bool WhitelistClaim(va_list Variadic)
{
    bool *Result = va_arg(Variadic, bool *);

    // This is an official plugin and should
    // be checked for modification and updates.
    *Result = true;

    return true;
}
bool RecvMessage(va_list Variadic)
{
    const char *Sender = va_arg(Variadic, const char *);
    size_t Command = va_arg(Variadic, size_t);
    const char *Message = va_arg(Variadic, const char *);

    switch (Command)
    {
    case FNV1a_Compiletime("NoneImplemented"):
    default:
        DebugPrint(va("%s from \"%s\" with message \"%s\".", __func__, Sender, Message));
        return false;
    }

    return true;
}

extern "C"
{
    // Plugin initialization.
    EXPORTSTART(PluginLoading);
    EXPORTFUNCTION("PreInitialization", PreInitialization);
    EXPORTFUNCTION("Shutdown", Shutdown);
    EXPORTEND;

    // Fetching information about a plugin.
    EXPORTSTART(PluginInformation);
    EXPORTFUNCTION("AuthorName", AuthorName);               // char *Result
    EXPORTFUNCTION("AuthorSite", AuthorSite);               // char *Result
    EXPORTFUNCTION("InternalName", InternalName);           // char *Result
    EXPORTFUNCTION("OnDiskHash", OnDiskHash);               // size_t *Result
    EXPORTFUNCTION("DependencyCount", DependencyCount);     // size_t *Result
    EXPORTFUNCTION("DependencyName", DependencyName);       // size_t Index, char *Result
    EXPORTFUNCTION("WhitelistClaim", WhitelistClaim);       // bool *Result
    EXPORTEND;

    // Interplugin communication.
    EXPORTSTART(PluginCommunication);
    EXPORTFUNCTION("RecvMessage", RecvMessage);             // const char *Pluginname, size_t Command, const char *Message
    EXPORTEND;
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
        PluginLoading(FNV1a_Compiletime("Shutdown"));
        break;
    }

    return TRUE;
}
