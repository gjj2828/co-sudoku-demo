#ifndef __ISOCKOBJ_H__
#define __ISOCKOBJ_H__

#include <Packet.h>

class ISockObj
{
public:
    virtual int     GetId()                                                                                                         = 0;
    virtual int     Listen(SOCKADDR* addr, int namelen, int buf_len, int backlog = 5, bool is_broadcast = false)                    = 0;
    virtual int     Accept(SOCKET sock)                                                                                             = 0;
    virtual int     Connect(SOCKADDR* remote_addr, int remote_namelen, SOCKADDR* local_addr, int local_namelen, char* buf, int len) = 0;
    virtual int     Send(Packet* packet, SOCKADDR* addr = NULL, int namelen = 0)                                                    = 0;
    virtual int     Update()                                                                                                        = 0;
    virtual void    Close()                                                                                                         = 0;
    virtual void    Release()                                                                                                       = 0;
};

#endif // __ISOCKOBJ_H__