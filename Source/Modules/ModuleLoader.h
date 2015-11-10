/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-09
    Notes:
        Modules are read from a CSV file with hostname.
        Formated as:
        Hostname, Modulename, Modulelicense
*/

#pragma once

struct ModuleLoader
{
    // Load a module into an internal array.
    static bool LoadModule(const char *Modulename, const char *License);

    // Load modules from a CSV, uses LoadModule.
    static bool LoadFromCSV(const char *CSVName);

    // Create a server instance from the module.
    static class IServer *CreateInstance(const char *Modulename, const char *Hostname);
};
