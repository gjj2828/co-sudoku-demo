#ifndef __INETWORKEVENTMANAGER_H__
#define __INETWORKEVENTMANAGER_H__

#include <Packet.h>

class ISockObj;

class INetworkEventManager
{
public:
    enum EEvent
    {
        EEVENT_MIN,
        EEVENT_ONACCEPT = EEVENT_MIN,
        EEVENT_ONCONNECT,
        EEVENT_ONSEND,
        EEVENT_ONRECV,
        EEVENT_CLOSE,
        EEVENT_WRONGSOCKTYPE,
        EEVENT_SOCKETALREADEXIST,
        EEVENT_INVALIDSOCKET,
        EEVENT_CREATEFAIL,
        EEVENT_BINDFAIL,
        EEVENT_LISTENFAIL,
        EEVENT_SETBROADCASTFAIL,
        EEVENT_POSTCONNECTFAIL,
        EEVENT_POSTACCEPTFAIL,
        EEVENT_POSTSENDFAIL,
        EEVENT_POSTRECVFAIL,
        EEVENT_ONACCEPTFAIL,
		EEVENT_ONCONNECTFAIL,
		EEVENT_ONSENDFAIL,
		EEVENT_ONRECVFAIL,
        EEVENT_MAX,
    };
    struct Event
    {
        ISockObj*   pSockObj;
        EEvent      eEvent;
        int         iRetCode;
        SOCKADDR*   pLocalAddr;
        SOCKADDR*   pRemoteAddr;
        Packet*     pPacket;
        SOCKET      soAccept;
        char*       pBuf;
        int         iBufLen;
    };
    virtual int HandleEvent(const Event& event) = 0;
};

#endif // __INETWORKEVENTMANAGER_H__