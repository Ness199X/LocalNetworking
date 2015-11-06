/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-11-06
    Notes:
        A basic datatype representing an object that can be serialized.
        This datatype is base for most data sent.
*/

#pragma once

struct ISerializable
{
    virtual void Serialize(class ByteBuffer *Buffer);
    virtual void Deserialize(class ByteBuffer *Buffer);
};

inline void ISerializable::Serialize(class ByteBuffer *Buffer)
{

}
inline void ISerializable::Deserialize(class ByteBuffer *Buffer)
{
}
