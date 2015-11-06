/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-06
    Notes:
        Each module needs to implement their own platform independent methods.
        Servers are identified by a hash of the hostname.
        Servers must manage their own dataqueues.
*/

#pragma once
#include <string>
#include "..\Utility\Crypto\FNV1.h"

// Index defines that should be expanded in the module defining a server.
#define SERVER_BEGIN 0
#define SERVER_END   31
#define SERVER_HOST  0

class IServer
{
protected:
    // Platform independent methods.
    virtual int32_t Platform_Disconnect(void *Socket) = 0;
    virtual int32_t Platform_Connect(void *Socket, void *NameStruct, int32_t NameLength) = 0;
    virtual int32_t Platform_Send(uint8_t *Buffer, int32_t Length, void *Socket = nullptr) = 0;
    virtual int32_t Platform_Receive(uint8_t *Buffer, int32_t Length, void *Socket = nullptr) = 0;
    virtual void    Platform_Select(int32_t *ReadCount, void **SocketsRead, int32_t *WriteCount, void **SocketsWrite) = 0;

    // Expose platform_ methods to the platform manager.
    friend class NTServerManager;
    friend class NIXServerManager;

public:
    // Internal storage as methods will be stateless.
    unsigned char LocalStorage[32];

    // Identifiers and debug information.
    size_t Identifier;
    const char *Hostname;

    // Usercode access to push data.
    virtual int32_t EnqueueData(std::string &Data) = 0;
    virtual int32_t EnqueueData(int32_t Size, void *Data) = 0;

    // Constructors.
    IServer();
    IServer(char *Hostname);
};
inline IServer::IServer()
{
    Identifier = 0;
}
inline IServer::IServer(char *Hostname)
{
    Identifier = FNV1a_Runtime(Hostname, sizeof(Hostname));
}
