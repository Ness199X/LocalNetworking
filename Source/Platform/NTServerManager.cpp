/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-07
    Notes:
        Abstraction of the platforms networking interface.
        If a call can't be handled by a IServer, forward it to the platform.
*/

#include "NTServerManager.h"
#include "..\Utility\Binary\Hooking.h"
#include "..\Macros.h"
#include <chrono>

// Internal map of bound IServer instances for easier management.
std::unordered_map<size_t /* Identifier */, class IServer *> NTServerManager::ServerAddresses;
std::unordered_map<size_t /* Socket */, class IServer *>     NTServerManager::ServerSockets;
std::unordered_map<size_t /* Socket */, bool /* Blocked */>  NTServerManager::BlockStatus;

// Internal map lookup methods.
class IServer *NTServerManager::FetchServerByAddress(size_t Address)
{
    auto Result = ServerAddresses.find(Address);
    return Result == ServerAddresses.end() ? nullptr : Result->second;
}
class IServer *NTServerManager::FetchServerBySocket(size_t Socket)
{
    auto Result = ServerSockets.find(Socket);
    return Result == ServerSockets.end() ? nullptr : Result->second;
}

// Initialization and modification of the platform layer.
bool NTServerManager::InitializeWinsockHooks()
{
#define PATCH_WINSOCK_IAT(name, function) \
    PatchCount += RedirectIAT("wsock32.dll", name, function) != 0 ? 1 : 0; \
    PatchCount += RedirectIAT("WS2_32.dll", name, function)  != 0 ? 1 : 0; 

    uint32_t PatchCount = 0;

    PATCH_WINSOCK_IAT("accept", NT_Accept);
    PATCH_WINSOCK_IAT("bind", NT_Bind);
    PATCH_WINSOCK_IAT("closesocket", NT_CloseSocket);
    PATCH_WINSOCK_IAT("connect", NT_Connect);
    PATCH_WINSOCK_IAT("getpeername", NT_GetPeerName);
    PATCH_WINSOCK_IAT("getsockname", NT_GetSockName);
    PATCH_WINSOCK_IAT("getsockopt", NT_GetSockOpt);
    PATCH_WINSOCK_IAT("ioctlsocket", NT_IOControlSocket);
    PATCH_WINSOCK_IAT("listen", NT_Listen);
    PATCH_WINSOCK_IAT("recv", NT_Receive);
    PATCH_WINSOCK_IAT("recvfrom", NT_ReceiveFrom);
    PATCH_WINSOCK_IAT("select", NT_Select);
    PATCH_WINSOCK_IAT("send", NT_Send);
    PATCH_WINSOCK_IAT("sendto", NT_SendTo);
    PATCH_WINSOCK_IAT("setsockopt", NT_SetSockOpt);
    PATCH_WINSOCK_IAT("gethostbyname", NT_GetHostByName);

    return PatchCount > 0;
}
void NTServerManager::InitializeServerInterface(class IServer *Server)
{
    ServerAddresses.emplace(Server->Identifier, Server);
}
void NTServerManager::SendShutdown()
{
    for (auto Iterator = ServerAddresses.begin(); Iterator != ServerAddresses.end(); ++Iterator)
        Iterator->second->Platform_Disconnect(-1);

    ServerAddresses.clear();
}

// Methods that replace the original winsock imports.
size_t __stdcall     NTServerManager::NT_Accept(size_t Socket, sockaddr *Address, int32_t *AddressLength)
{
    return accept(Socket, Address, AddressLength);
}
int32_t __stdcall    NTServerManager::NT_Bind(size_t Socket, const sockaddr *Address, int32_t AddressLength)
{
    if (AddressLength == sizeof(sockaddr_in))
    {
        NetworkPrint(va("%s to address %s:%u on socket 0x%X", __func__, inet_ntoa(((sockaddr_in *)Address)->sin_addr), ntohs(((sockaddr_in *)Address)->sin_port), Socket));
    }

    return bind(Socket, Address, AddressLength);
}
int32_t __stdcall    NTServerManager::NT_CloseSocket(size_t Socket)
{
    class IServer *Server = nullptr;

    // Find the server if we have one.
    Server = FetchServerBySocket(Socket);
    if (Server != nullptr)
    {
        NetworkPrint(va("%s for server \"%s\"", __func__, Server->Hostname));
        Server->Platform_Disconnect(Socket);
        ServerSockets.erase(Socket);
    }

    return closesocket(Socket);
}
int32_t __stdcall    NTServerManager::NT_Connect(size_t Socket, const sockaddr *Address, int32_t AddressLength)
{
    class IServer *Server = nullptr;

    // Find the server if we have one.
    Server = FetchServerBySocket(Socket);
    if (Server != nullptr)
    {
        Server->Platform_Disconnect(Socket);
        ServerSockets.erase(Socket);
    }
    Server = FetchServerByAddress(((sockaddr_in *)Address)->sin_addr.s_addr);
        
    // Let winsock handle the request if we can't.
    if (Server == nullptr)
    {
        NetworkPrint(va("%s to address %s:%u on socket 0x%X", __func__, inet_ntoa(((sockaddr_in *)Address)->sin_addr), ntohs(((sockaddr_in *)Address)->sin_port), Socket));
        return connect(Socket, Address, AddressLength);
    }

    // Add the socket to the map.
    ServerSockets.emplace(Socket, Server);

    NetworkPrint(va("%s to address %s:%u on socket 0x%X", __func__, inet_ntoa(((sockaddr_in *)Address)->sin_addr), ntohs(((sockaddr_in *)Address)->sin_port), Socket));
    return Server->Platform_Connect(Socket, (void *)Address, AddressLength);
}
int32_t __stdcall    NTServerManager::NT_GetPeerName(size_t Socket, sockaddr *Address, int32_t *AddressLength)
{
    return getpeername(Socket, Address, AddressLength);
}
int32_t __stdcall    NTServerManager::NT_GetSockName(size_t Socket, sockaddr *Address, int32_t *AddressLength)
{
    return getsockname(Socket, Address, AddressLength);
}
int32_t __stdcall    NTServerManager::NT_GetSockOpt(size_t Socket, int32_t Level, int32_t OptionName, char *OptionValue, int32_t *OptionLength)
{
    return getsockopt(Socket, Level, OptionName, OptionValue, OptionLength);
}
int32_t __stdcall    NTServerManager::NT_IOControlSocket(size_t Socket, uint32_t Command, u_long *ArgumentPointer)
{
    const char *ReadableCommand = "UNKNOWN";

    switch (Command)
    {
    case FIONBIO: ReadableCommand = "FIONBIO"; 
        // Set the blocking status.
        BlockStatus[Socket] = *ArgumentPointer == 0;
        break;
    case FIONREAD: ReadableCommand = "FIONREAD"; break;
    case FIOASYNC: ReadableCommand = "FIOASYNC"; break;

    case SIOCSHIWAT: ReadableCommand = "SIOCSHIWAT"; break;
    case SIOCGHIWAT: ReadableCommand = "SIOCGHIWAT"; break;
    case SIOCSLOWAT: ReadableCommand = "SIOCSLOWAT"; break;
    case SIOCGLOWAT: ReadableCommand = "SIOCGLOWAT"; break;
    case SIOCATMARK: ReadableCommand = "SIOCATMARK"; break;
    }

    NetworkPrint(va("%s on socket 0x%X with command \"%s\"", __func__, Socket, ReadableCommand));
    return ioctlsocket((SOCKET)Socket, Command, ArgumentPointer);
}
int32_t __stdcall    NTServerManager::NT_Listen(size_t Socket, int32_t Backlog)
{
    return listen(Socket, Backlog);
}
int32_t __stdcall    NTServerManager::NT_Receive(size_t Socket, char *Buffer, int32_t BufferLength, int32_t Flags)
{
    class IServer *Server = nullptr;
    int32_t BytesReceived = -1;

    // Find the server if we have one.
    Server = FetchServerBySocket(Socket);

    // Let winsock handle the request if we can't.
    if (Server == nullptr)
    {
        BytesReceived = recv(Socket, Buffer, BufferLength, Flags);
        if(BytesReceived != -1)
            NetworkPrint(va("%s via 0x%X, %i bytes.", __func__, Socket, BytesReceived));
        return BytesReceived;
    }

    // If this socket is blocking, wait until we have any data to send.
    do
    {
        BytesReceived = Server->Platform_Receive((uint8_t *)Buffer, BufferLength, Socket);
        if (BytesReceived == -1) 
            // Add 30ms of artificial ping as some applications get scared of low latency.
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
    } while (BlockStatus[Socket] && BytesReceived == -1);

    // If we are non-blocking, set last error.
    if (!BlockStatus[Socket] && BytesReceived == -1)
        WSASetLastError(WSAEWOULDBLOCK);

    // Log how much data we received.
    if (BytesReceived != -1)
        NetworkPrint(va("%s: %i bytes from %s", __func__, BytesReceived, Server->Hostname));

    return BytesReceived;
}
int32_t __stdcall    NTServerManager::NT_ReceiveFrom(size_t Socket, char *Buffer, int32_t BufferLength, int32_t Flags, sockaddr *Peer, int32_t *PeerLength)
{
    class IServer *Server = nullptr;
    int32_t BytesReceived = -1;

    // Find the server if we have one.
    Server = FetchServerBySocket(Socket);
    if (Server == nullptr)
        Server = FetchServerByAddress(((sockaddr_in *)Peer)->sin_addr.s_addr);

    // Let winsock handle the request if we can't.
    if (Server == nullptr)
    {
        BytesReceived = recvfrom(Socket, Buffer, BufferLength, Flags, Peer, PeerLength);
        if(BytesReceived != -1)
            NetworkPrint(va("%s via 0x%X, %i bytes.", __func__, Socket, BytesReceived));
        return BytesReceived;
    }

    // Set the host information from previous UDP calls.
    memcpy(Peer, &Server->LocalStorage[SERVER_HOST], sizeof(sockaddr));

    // If this socket is blocking, wait until we have any data to send.
    do
    {
        BytesReceived = Server->Platform_Receive((uint8_t *)Buffer, BufferLength, Socket);
        if (BytesReceived == -1) 
            // Add 30ms of artificial ping as some applications get scared of low latency.
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
    } while (BlockStatus[Socket] && BytesReceived == -1);

    // If we are non-blocking, set last error.
    if (!BlockStatus[Socket] && BytesReceived == -1)
        WSASetLastError(WSAEWOULDBLOCK);
        
    // Log how much data we received.
    if (BytesReceived != -1)
        NetworkPrint(va("%s: %i bytes from %s", __func__, BytesReceived, Server->Hostname));
    
    return BytesReceived;
}
int32_t __stdcall    NTServerManager::NT_Select(int32_t fdsCount, fd_set *Readfds, fd_set *Writefds, fd_set *Exceptfds, const timeval *Timeout)
{
    int32_t SocketCount;
    int32_t ReadCount, WriteCount;
    size_t ReadSockets[10], WriteSockets[10];

    // Request socket info from winsock.
    SocketCount = select(fdsCount, Readfds, Writefds, Exceptfds, Timeout);
    if (SocketCount == -1)
        SocketCount = 0;

    // Add our servers sockets.
    for (auto Iterator = ServerSockets.begin(); Iterator != ServerSockets.end(); ++Iterator)
    {
        WriteCount = 10; ReadCount = 10;
        Iterator->second->Platform_Select(&ReadCount, ReadSockets, &WriteCount, WriteSockets);

        if (Readfds)
        {
            for (int32_t i = 0; i < ReadCount; ++i)
            {
                FD_SET(ReadSockets[i], Readfds);
                SocketCount++;
            }
        }
        if (Writefds)
        {
            for (int32_t i = 0; i < WriteCount; ++i)
            {
                FD_SET(WriteSockets[i], Writefds);
                SocketCount++;
            }
        }
    }

    // Return the total number of updated sockets.
    return SocketCount;
}
int32_t __stdcall    NTServerManager::NT_Send(size_t Socket, const char *Buffer, int32_t BufferLength, int32_t Flags)
{
    class IServer *Server = nullptr;

    // Find the server if we have one.
    Server = FetchServerBySocket(Socket);

    // Let winsock handle the request if we can't.
    if (Server == nullptr)
    {
        NetworkPrint(va("%s via 0x%X, %i bytes.", __func__, Socket, BufferLength));
        return send(Socket, Buffer, BufferLength, Flags);
    }

    NetworkPrint(va("%s to \"%s\", %i bytes.", __func__, Server->Hostname, BufferLength));
    return Server->Platform_Send((uint8_t *)Buffer, BufferLength, Socket);
}
int32_t __stdcall    NTServerManager::NT_SendTo(size_t Socket, const char *Buffer, int32_t BufferLength, int32_t Flags, const sockaddr *Peer, int32_t PeerLength)
{
    class IServer *Server = nullptr;

    // Find the server if we have one.
    Server = FetchServerBySocket(Socket);
    if (Server == nullptr)
        Server = FetchServerByAddress(((sockaddr_in *)Peer)->sin_addr.s_addr);

    // Let winsock handle the request if we can't.
    if (Server == nullptr)
    {
        NetworkPrint(va("%s via 0x%X, %i bytes.", __func__, Socket, BufferLength));
        return sendto(Socket, Buffer, BufferLength, Flags, Peer, PeerLength);
    }

    // Add the socket to the internal map and save the host info.
    ServerSockets.emplace(Socket, Server);
    memcpy(&Server->LocalStorage[SERVER_HOST], Peer, sizeof(sockaddr));

    NetworkPrint(va("%s to \"%s\", %i bytes.", __func__, Server->Hostname, BufferLength));
    return Server->Platform_Send((uint8_t *)Buffer, BufferLength, Socket);
}
int32_t __stdcall    NTServerManager::NT_SetSockOpt(size_t Socket, int32_t Level, int32_t OptionName, const char *OptionValue, int32_t OptionLength)
{
    return setsockopt(Socket, Level, OptionName, OptionValue, OptionLength);
}
hostent *__stdcall   NTServerManager::NT_GetHostByName(const char *Hostname)
{
    class IServer *Server = nullptr;

    // Find the server if we have one.
    if (INADDR_NONE == inet_addr(Hostname))
        Server = FetchServerByAddress(FNV1a_Runtime((char *)Hostname, sizeof(Hostname)));
    else
        Server = FetchServerByAddress(inet_addr(Hostname));

    // Let winsock handle the request if we can't.
    if (Server == nullptr)
    {
        static hostent *ResolvedHost = nullptr;
        ResolvedHost = gethostbyname(Hostname);

        // Debug information about winsocks result.
        if (ResolvedHost != nullptr)
            NetworkPrint(va("%s: \"%s\" -> %s", __func__, Hostname, inet_ntoa(*(in_addr*)ResolvedHost->h_addr_list[0])));

        return ResolvedHost;
    }

    // Create the local address.
    static in_addr LocalAddress;
    static in_addr *LocalSocketAddrList[2];
    LocalAddress.s_addr = Server->Identifier;
    LocalSocketAddrList[0] = &LocalAddress;
    LocalSocketAddrList[1] = nullptr;

    static hostent LocalHost;
    LocalHost.h_name = const_cast<char *>(Hostname);
    LocalHost.h_aliases = NULL;
    LocalHost.h_addrtype = AF_INET;
    LocalHost.h_length = sizeof(in_addr);
    LocalHost.h_addr_list = (char **)LocalSocketAddrList;

    NetworkPrint(va("%s: \"%s\" -> %s", __func__, Hostname, inet_ntoa(LocalAddress)));
    return &LocalHost;
}
