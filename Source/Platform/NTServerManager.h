/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-07
    Notes:
        Abstraction of the platforms networking interface.
        If a call can't be handled by a IServer, forward it to the platform.
*/

#pragma once
#include "..\Interfaces\IServer.h"
#include <unordered_map>

class NTServerManager
{
    // Internal map of bound IServer instances for easier management.
    static std::unordered_map<size_t /* Identifier */, class IServer *> ServerAddresses;
    static std::unordered_map<size_t /* Socket */, class IServer *>     ServerSockets;
    static std::unordered_map<size_t /* Socket */, bool /* Blocked */>  BlockStatus;

    // Internal map lookup methods.
    static class IServer *FetchServerByAddress(size_t Address);
    static class IServer *FetchServerBySocket(size_t Socket);

    // Methods that replace the original winsock imports.
    static size_t __stdcall     NT_Accept(size_t Socket, sockaddr *Address, int32_t *AddressLength);
    static int32_t __stdcall    NT_Bind(size_t Socket, const sockaddr *Address, int32_t AddressLength);
    static int32_t __stdcall    NT_CloseSocket(size_t Socket);
    static int32_t __stdcall    NT_Connect(size_t Socket, const sockaddr *Address, int32_t AddressLength);
    static int32_t __stdcall    NT_GetPeerName(size_t Socket, sockaddr *Address, int32_t *AddressLength);
    static int32_t __stdcall    NT_GetSockName(size_t Socket, sockaddr *Address, int32_t *AddressLength);
    static int32_t __stdcall    NT_GetSockOpt(size_t Socket, int32_t Level, int32_t OptionName, char *OptionValue, int32_t *OptionLength);
    static int32_t __stdcall    NT_IOControlSocket(size_t Socket, uint32_t Command, u_long *ArgumentPointer);
    static int32_t __stdcall    NT_Listen(size_t Socket, int32_t Backlog);
    static int32_t __stdcall    NT_Receive(size_t Socket, char *Buffer, int32_t BufferLength, int32_t Flags);
    static int32_t __stdcall    NT_ReceiveFrom(size_t Socket, char *Buffer, int32_t BufferLength, int32_t Flags, sockaddr *Peer, int32_t *PeerLength);
    static int32_t __stdcall    NT_Select(int32_t fdsCount, fd_set *Readfds, fd_set *Writefds, fd_set *Exceptfds, const timeval *Timeout);
    static int32_t __stdcall    NT_Send(size_t Socket, const char *Buffer, int32_t BufferLength, int32_t Flags);
    static int32_t __stdcall    NT_SendTo(size_t Socket, const char *Buffer, int32_t BufferLength, int32_t Flags, const sockaddr *Peer, int32_t PeerLength);
    static int32_t __stdcall    NT_SetSockOpt(size_t Socket, int32_t Level, int32_t OptionName, const char *OptionValue, int32_t OptionLength);
    static hostent *__stdcall   NT_GetHostByName(const char *Hostname);

public:
    // Initialization and modification of the platform layer.
    static bool InitializeWinsockHooks();
    static void InitializeServerInterface(class IServer *Server);
};
