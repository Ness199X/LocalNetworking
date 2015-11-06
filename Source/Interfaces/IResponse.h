/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-06
    Notes:
        The response class takes an array of serializable objects
        and when ready, serializes and sends them to the server.
*/

#pragma once
#include <vector>
#include <memory>
#include "ISerializable.h"

// Index defines that should be expanded in the module defining a response.
#define RESPONSE_BEGIN 0
#define RESPONSE_END   31

class IResponse
{
protected:
    // The server to send to and the array of objects.
    std::vector<std::shared_ptr<struct ISerializable>> ObjectBuffer;
    class IServer *ParentServer;

public:
    // Internal storage as responses may require additional information.
    unsigned char LocalStorage[32];

    // Expand the buffer and ship it.
    virtual size_t Enqueue(std::shared_ptr<struct ISerializable> Object);
    virtual void   Clear();
    virtual size_t Send() = 0;

    // Constructor.
    IResponse();
    IResponse(class IServer *Parent);
};

inline size_t IResponse::Enqueue(std::shared_ptr<struct ISerializable> Object)
{
    ObjectBuffer.push_back(Object);
    return ObjectBuffer.size();
}
inline void IResponse::Clear()
{
    ObjectBuffer.clear();
    ObjectBuffer.shrink_to_fit();
}
inline IResponse::IResponse()
{
    ParentServer = nullptr;
}
inline IResponse::IResponse(class IServer *Parent)
{
    ParentServer = Parent;
}
