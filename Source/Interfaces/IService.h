/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-06
    Notes:
        A service is intended to be a submodule of a server interface.
        As such they have an identifying value and can handle a task.
        Example: ((IService*) OnPacketRecived)->CallService(Packet);
*/

#pragma once
#include <string>

// Index defines that should be expanded in the module defining a service.
#define SERVICE_TASKID 0
#define SERVICE_RESULT 4

struct IService
{
    // Internal storage as methods will be stateless.
    unsigned char LocalStorage[16];

    // Service information and handler. Handler returns if the data was handled.
    virtual size_t ServiceID() = 0;
    virtual bool HandlePacket(class IServer *Caller, std::string &PacketData) = 0;
};
