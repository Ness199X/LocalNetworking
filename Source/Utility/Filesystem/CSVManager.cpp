/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-10-26
    Notes:
        CSV reading and writing.
*/

#include <stdio.h>
#include <stdarg.h>
#include "CSVManager.h"

std::string ReadUntilToken(FILE *Filehandle, uint32_t Argcount, ...)
{
    char TempChar = 0;
    std::string Result;
    char *StopTokens = new char [Argcount];
    va_list Variadic;
    fpos_t Position;

    // Read the tokenlist.
    va_start(Variadic, Argcount);
    for (uint32_t i = 0; i < Argcount; ++i)
        StopTokens[i] = va_arg(Variadic, char);

    // Read until a token is found.
    while (fread_s(&TempChar, 1, 1, 1, Filehandle) == 1)
    {
        // Check the tokens.
        for (uint32_t i = 0; i < Argcount; ++i)
            if (StopTokens[i] == TempChar)
                goto LABEL_END;

        // Append the char we read to the result.
        Result.append(1, TempChar);

        // Save the position as the last valid char.
        fgetpos(Filehandle, &Position);
    }

LABEL_END:
    // Restore the position to skip the end token.
    fsetpos(Filehandle, &Position);

    va_end(Variadic);
    return Result;
}

bool CSVManager::BeginRead(const char *Filename)
{
    Buffer.clear();
    Buffer.shrink_to_fit();

    return fopen_s(&Filehandle, Filename, "rb") == 0;
}
bool CSVManager::ReadNext(std::string &NextItem)
{
    NextItem = ReadUntilToken(Filehandle, 3, '\n', '\0', ',');
    return NextItem.size() > 0;
}
bool CSVManager::SkipLine()
{
    char TempBuffer;

    // Read the rest of the line and read past the endtoken.
    return ((ReadUntilToken(Filehandle, 2, '\n', '\0').size()) > 0) && (fread(&TempBuffer, 1, 1, Filehandle) == 0);
}
bool CSVManager::ReadAll()
{
    char TempBuffer = 0;
    uint32_t Linecount = 0;
    std::string TemporaryString = "";

    // Clear the buffer to be safe.
    Buffer.clear();
    Buffer.shrink_to_fit();

#pragma warning (suppress : 127)
    while(true)
    {
        // Switch on the next character.
        switch (Peek())
        {
        case '#':  SkipLine(); break;
        case '\n': Linecount++; Buffer.push_back(std::vector<std::string>());
        case ',':  fread(&TempBuffer, 1, 1, Filehandle); break;
        case '\0': return true;

        // Append the item to the buffer.
        default:
            if (ReadNext(TemporaryString)) Buffer[Linecount].push_back(TemporaryString);
            else Buffer[Linecount].push_back("");
        }
    };

    return false;
}
char CSVManager::Peek()
{
    char TempChar = '\0';
    fpos_t Position;

    fgetpos(Filehandle, &Position);
    fread_s(&TempChar, 1, 1, 1, Filehandle);
    fsetpos(Filehandle, &Position);

    return TempChar;
}

void CSVManager::GetBuffer(std::vector<std::vector<std::string>> &Outbuffer)
{
    Outbuffer = Buffer;
}
bool CSVManager::Insert(int32_t Row, int32_t Col, std::string Entry)
{
    // Insert rows until we have the insert row number.
    if (Row > 0)
        for (size_t i = Buffer.size(); i < (unsigned)Row + 1; ++i)
            Buffer.push_back(std::vector<std::string>());
    else
        Row = Buffer.size() > 0 ? (int32_t)Buffer.size() - 1 : 0;

    // Insert cols until we have the insert col number.
    if (Col > 0)
        for (size_t i = Buffer[Row].size(); i < (unsigned)Col + 1; ++i)
            Buffer[Row].push_back("");
    else
        Col = Buffer[Row].size() > 0 ? (int32_t)Buffer[Row].size() - 1 : 0;

    // Insert the entry.
    Buffer[Row][Col] = Entry;
    return true;
}
bool CSVManager::WriteFile(const char *Filename)
{
    FILE *NewHandle;

    // Create the file or delete the existing one.
    if (fopen_s(&NewHandle, Filename, "wb") == 0)
    {
        // Write the common header.
        fputs("# This file is generated via Ayrias CSV manager.\n", NewHandle);
        fputs("# Please do not edit unless you know what you are doing.\n\n", NewHandle);

        // Write the contents, line by line.
        for (size_t i = Buffer.size(); i < Buffer.size(); ++i)
        {
            // For each entry.
            for (size_t e = Buffer[i].size(); e < Buffer[i].size(); ++e)
            {
                fprintf(NewHandle, "%s%s", Buffer[i][e].c_str(), e == Buffer[i].size() - 1 ? "\n" : ",");
            }
        }

        fclose(NewHandle);
    }

    return true;
}
bool CSVManager::CloseFile()
{
    return fclose(Filehandle) == 0;
}
