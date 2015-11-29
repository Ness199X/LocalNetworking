/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-09
    Notes:
        Modules are read from a CSV file with hostname.
        Formated as:
        Hostname, Modulename, Modulelicense
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include <string>
#include "MemoryModule.h"
#include "ModuleLoader.h"
#include "..\Utility\String\VariadicString.h"
#include "..\Utility\Filesystem\CSVManager.h"
#include "..\Platform\NTServerManager.h"
#include "..\Utility\Compression\lz4.h"
#include "..\Utility\Crypto\AES256.h"
#include "..\Utility\Crypto\SM3.h"
#include "..\Macros.h"

// Representation of a module in the array.
struct LocalModule
{
    std::string Modulename;
    std::string ModuleData;

    union
    {
        void *DiskHandle;
        void *MemoryHandle;
    } ModuleHandle { nullptr };
    bool Memorymodule;
};

// Internal array of modules.
std::vector<LocalModule> ModuleArray;

// Load a module into an internal array.
bool ModuleLoader::LoadModule(const char *Modulename, const char *License)
{
    LocalModule NewModule;

    // Check for duplicates in the array.
    for (auto Iterator = ModuleArray.begin(); Iterator != ModuleArray.end(); ++Iterator)
        if (0 == _stricmp(Iterator->Modulename.c_str(), Modulename))
            return true;

    // If we get anything but a plain DLL, we load it from memory so read it into the buffer.
    char *Filebuffer;
    FILE *Filehandle;
    size_t Filesize;
    NewModule.Memorymodule = (strstr(Modulename, ".LZ4") || strstr(Modulename, ".AES") || !strstr(Modulename, ".dll"));
    if (NewModule.Memorymodule)
    {
        if (0 == fopen_s(&Filehandle, va("Plugins\\LocalNetworking\\Modules\\%s", Modulename), "rb"))
        {
            // Read the size.
            fseek(Filehandle, 0, SEEK_END);
            Filesize = ftell(Filehandle);
            fseek(Filehandle, 0, SEEK_SET);

            // Read the file into the buffer.
            Filebuffer = new char[Filesize + 1]();
            if (fread_s(Filebuffer, Filesize + 1, 1, Filesize, Filehandle) == Filesize)
                NewModule.ModuleData.append(Filebuffer, Filesize);

            delete[] Filebuffer;
            fclose(Filehandle);
        }
        else
        {
            DebugPrint(va("%s failed to load module \"%s\"", __func__, Modulename));
            return false;
        }
    }

    // Decrypt the module using the license provided.
    if (strstr(Modulename, ".AES"))
    {
        uint8_t EncryptionKey[24]{};
        uint8_t InitialVector[24]{};
        ByteArray Plaintext;

        // Use the default license if none is provided.
        if (License == nullptr) License = "NoLicense";

        // Create the encryption key and IV.
        sm3((unsigned char *)Modulename, strlen(Modulename), InitialVector);
        sm3((unsigned char *)License, strlen(License), EncryptionKey);

        // Decrypt the module.
        Aes256 aes(EncryptionKey, InitialVector);
        aes.decrypt_start(NewModule.ModuleData.size());
        aes.decrypt_continue((unsigned char *)NewModule.ModuleData.data(), NewModule.ModuleData.size(), Plaintext);
        aes.decrypt_end(Plaintext);

        // Replace the modules cipertext with plaintext.
        NewModule.ModuleData.clear();
        NewModule.ModuleData.append((char *)Plaintext.data(), Plaintext.size());
    }

    // Decompress the module using LZ4.
    if (strstr(Modulename, ".LZ4"))
    {
        char *InflatedBuffer;
        int InflatedLength = 0;

        // LZ4 has a compression ration of around 2.1, so we allocate 3 times the data.
        InflatedBuffer = new char[NewModule.ModuleData.size() * 3]();
        InflatedLength = LZ4_decompress_safe(NewModule.ModuleData.data(), InflatedBuffer, NewModule.ModuleData.size(), NewModule.ModuleData.size() * 3);

        // If the inflation failed we quit.
        if (InflatedLength < NewModule.ModuleData.size())
        {
            delete[] InflatedBuffer;
            return false;
        }

        // Replace the modules buffer.
        NewModule.ModuleData.clear();
        NewModule.ModuleData.append(InflatedBuffer, InflatedLength);
        delete[] InflatedBuffer;
    }

    // Load the module into memory.
    if (NewModule.Memorymodule)
        NewModule.ModuleHandle.MemoryHandle = MemoryLoadLibrary(NewModule.ModuleData.data());
    else
        NewModule.ModuleHandle.DiskHandle = LoadLibraryA(va("Plugins\\LocalNetworking\\Modules\\%s", Modulename));

    // Add the module to our array.
    NewModule.Modulename = Modulename;
    ModuleArray.push_back(NewModule);

    // NOTE: ModuleHandle is a union, any membercheck will work.
    return NewModule.ModuleHandle.DiskHandle != nullptr;
}

// Load modules from a CSV, uses LoadModule.
bool ModuleLoader::LoadFromCSV(const char *CSVName)
{
    CSVManager Manager;
    size_t InstanceCount = 0;    

    // Load the file into memory.
    if (!Manager.ReadFile(va("Plugins\\LocalNetworking\\%s", CSVName)))
    {
        DebugPrint(va("\n********* WARNING\nClould not load CSVfile \"%s\"\n********* WARNING\n", CSVName));
        return false;
    }

    // Load each module.
    for (size_t i = 0; i < Manager.EntryBuffer.size(); ++i)
        LoadModule(Manager.GetEntry(i, 1).c_str(), Manager.GetEntry(i, 2).c_str());

    // Create a new server instance for each entry.
    for (size_t i = 0; i < Manager.EntryBuffer.size(); ++i)
    {
        for (auto Moduleiterator = ModuleArray.begin(); Moduleiterator != ModuleArray.end(); ++Moduleiterator)
        {
            if (0 == strcmp(Moduleiterator->Modulename.c_str(), Manager.GetEntry(i, 1).c_str()))
            {
                static class IServer *Instance{ nullptr };
                Instance = CreateInstance(Moduleiterator->Modulename.c_str(), Manager.GetEntry(i, 0).c_str());

                if (Instance != nullptr)
                {
#ifdef _WIN32
                    NTServerManager::InitializeServerInterface(Instance);
#else
                    NIXServerManager::InitializeServerInterface(Instance);
#endif
                    InstanceCount++;
                }
            }
        }
    }

    // Debug information.
    InfoPrint(va("%s loaded %u modules and %u server instances.", __func__, ModuleArray.size(), InstanceCount));
    return InstanceCount > 0;
}

// Create a server instance from the module.
class IServer *ModuleLoader::CreateInstance(const char *Modulename, const char *Hostname)
{
    class IServer *(*_CreateInstance)(const char *) { nullptr };
    LocalModule *ThisModule{ nullptr };

    // Fetch the module we are looking for.
    for (auto Iterator = ModuleArray.begin(); Iterator != ModuleArray.end(); ++Iterator)
        if (0 == _stricmp(Iterator->Modulename.c_str(), Modulename))
            ThisModule = &(*Iterator);

    // Sanity check.
    if (ThisModule == nullptr)
    {
        DebugPrint(va("%s with an unloaded module; \"%s\"", __func__, Modulename));
        return nullptr;
    }

    // Find the exported procedure.
    if (ThisModule->Memorymodule)
        _CreateInstance = (class IServer *(*)(const char *))MemoryGetProcAddress(ThisModule->ModuleHandle.MemoryHandle, "CreateInstance");
    else
        _CreateInstance = (class IServer *(*)(const char *))GetProcAddress(GetModuleHandleA(ThisModule->Modulename.c_str()), "CreateInstance");

    // The module developer forgot to export the procedure.
    if (_CreateInstance == nullptr)
    {
        DebugPrint(va("%s with an ivalid module (missing export); \"%s\"", __func__, Modulename));
        return nullptr;
    }

    return _CreateInstance(Hostname);
}
