#ifndef __ISOCKOBJ_H__
#define __ISOCKOBJ_H__

#include <Packet.h>

class ISockObj
{
public:
    enum ESockType
    {
        ESOCKTYPE_MIN,
        ESOCKTYPE_TCP = ESOCKTYPE_MIN,
        ESOCKTYPE_UDP,
        ESOCKTYPE_MAX,
    };
    virtual int     GetId()                                 = 0;
    virtual void    SetId(int id)                           = 0;
    virtual int     Bind(const sockaddr* addr, int namelen) = 0;
    virtual int     Listen(int backlog)                     = 0;
    virtual int     Accept()                                = 0;
    virtual int     Connect()                               = 0;
    virtual int     Send(Packet* packet)                    = 0;
    virtual int     Recv()                                  = 0;
    virtual int     Update()                                = 0;
};

enum ESOResult
{
    ESORESULT_MIN,
    ESORESULT_ACCEPT = ESORESULT_MIN,
    ESORESULT_CONNECT,
    ESORESULT_RECV,
    ESORESULT_CLOSE,
    ESORESULT_FAIL,
    ESORESULT_MAX,
};

struct SOResult
{
    ESOResult   Result;
    ISockObj*   AcceptSO;
    Packet*     RecvPacket;
};

typedef int (*SOCallBack)(ISockObj*, const SOResult&);

#endif // __ISOCKOBJ_H__