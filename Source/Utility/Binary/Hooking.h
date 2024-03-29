/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-07
    Notes:
        Provides ways of redirecting calls in the host binary.
*/

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "..\String\VariadicString.h"
#include "..\..\Macros.h"

// Redirect an import in the host.
inline size_t RedirectIAT(const char *Modulename, const char *Functionname, void *NewFunction)
{
    size_t Imagebase;
    size_t OriginalEntry;
    PIMAGE_DOS_HEADER DOSHeader;
    PIMAGE_NT_HEADERS NTHeader;
    PIMAGE_IMPORT_DESCRIPTOR Imports;
    PIMAGE_THUNK_DATA ImportThunkData;

    // Initialize host information.
    Imagebase = (size_t)GetModuleHandleA(NULL);
    OriginalEntry = (size_t)GetProcAddress(GetModuleHandleA(Modulename), Functionname);

    // Fetch information from the PE header.
    DOSHeader = (PIMAGE_DOS_HEADER)Imagebase;
    NTHeader = (PIMAGE_NT_HEADERS)(DOSHeader->e_lfanew + Imagebase);

    // Verify that the import table has not been destroyed by a packer.
    if (NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0)
    {
        DebugPrint(va("%s: The current application does not have an import table.", __func__));
        return 0;
    }

    // Iterate through the import table until we find our enrty.
    Imports = (PIMAGE_IMPORT_DESCRIPTOR)((NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) + Imagebase);
    for (size_t i = 0; Imports[i].Characteristics != 0; ++i)
    {
        // Skip unrelated modules.
        if (_stricmp((char *)(Imports[i].Name + Imagebase), Modulename))
            continue;

        // Iterate through the thunks.
        for (size_t c = 0; ; ++c)
        {
            // Fetch the next thunk and verify that it's not the last.
            ImportThunkData = (PIMAGE_THUNK_DATA)((size_t)Imports[i].OriginalFirstThunk + (c * sizeof(IMAGE_THUNK_DATA)) + Imagebase);
            if (ImportThunkData->u1.AddressOfData == NULL)
                break;

            // Verify the import by ordinal.
            if (ImportThunkData->u1.Ordinal & IMAGE_ORDINAL_FLAG)
            {
                PIMAGE_THUNK_DATA OrdinalThunk = (PIMAGE_THUNK_DATA)((size_t)Imports[i].FirstThunk + (c * sizeof(IMAGE_THUNK_DATA)) + Imagebase);

                if (OriginalEntry == OrdinalThunk->u1.Function)
                {
                    OriginalEntry = OrdinalThunk->u1.Function;
                    OrdinalThunk->u1.Function = (size_t)NewFunction;
                    return OriginalEntry;
                }
                else
                    continue;
            }

            // Verify the import by name.
            if (!_stricmp(((PIMAGE_IMPORT_BY_NAME)(ImportThunkData->u1.AddressOfData + Imagebase))->Name, Functionname))
            {
                PIMAGE_THUNK_DATA NameThunk = (PIMAGE_THUNK_DATA)((size_t)Imports[i].FirstThunk + (c * sizeof(IMAGE_THUNK_DATA)) + Imagebase);
                OriginalEntry = NameThunk->u1.Function;
                NameThunk->u1.Function = (size_t)NewFunction;
                return OriginalEntry;
            }
        }
    }

    return 0;
}
