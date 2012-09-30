#ifndef __USERPACKET_H__
#define __USERPACKET_H__

#include <Packet.h>

enum EPacketType
{
    EPACKETTYPE_MIN,
    EPACKETTYPE_TEST = EPACKETTYPE_MIN,
    EPACKETTYPE_MAX,
};

struct TestPacket : public Packet
{
    TestPacket()
    {
        size = sizeof(*this);
        type = EPACKETTYPE_TEST;
    }

    int test;
};

#endif // __USERPACKET_H__