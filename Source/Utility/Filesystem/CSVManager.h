/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-10-26
    Notes:
        CSV reading and writing.
*/

#pragma once
#include <string>
#include <vector>

class CSVManager
{
    FILE *Filehandle;
    std::vector<std::vector<std::string>> Buffer;

public:
    bool BeginRead(const char *Filename);
    bool ReadNext(std::string &NextItem);
    bool SkipLine();
    bool ReadAll();
    char Peek();
    
    void GetBuffer(std::vector<std::vector<std::string>> &Outbuffer);
    bool Insert(int32_t Row, int32_t Col, std::string Entry);
    bool WriteFile(const char *Filename);
    bool CloseFile();
};

