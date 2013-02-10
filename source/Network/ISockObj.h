#ifndef __ISOCKOBJ_H__
#define __ISOCKOBJ_H__

#include <Packet.h>

class ISockObj
{
public:
    enum ESockType
    {
        ESOCKTYPE_MIN,
        ESOCKTYPE_TCP,
        ESOCKTYPE_UDP,
        ESOCKTYPE_MAX,
    };
    virtual int     GetId()                                                                                                         = 0;
    virtual int     Listen(ESockType type, SOCKADDR* addr = NULL, int namelen = 0, int buf_len = 0, int backlog = 5)                = 0;
    virtual int     Accept(SOCKET sock)                                                                                             = 0;
    virtual int     Connect(SOCKADDR* remote_addr, int remote_namelen, SOCKADDR* local_addr, int local_namelen, char* buf, int len) = 0;
    virtual int     Send(Packet* packet, SOCKADDR* addr = NULL, int namelen = 0)                                                    = 0;
    virtual int     Update()                                                                                                        = 0;
    virtual void    Close()                                                                                                         = 0;
    virtual void    Release()                                                                                                       = 0;
};

#endif // __ISOCKOBJ_H__