#ifndef INETWORKHANDLER_H_
#define INETWORKHANDLER_H_

#include <cstddef>

/** implementers may get notified about network events */
class INetworkHandler
{
public:
    virtual ~INetworkHandler() {}

    virtual void onNetworkReadComplete(const char* msg, std::size_t length) = 0;
    virtual void onNetworkClientAccept() = 0;
    virtual void onNetworkClientDisconnect() = 0;
};


#endif
