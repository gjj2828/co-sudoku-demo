#ifndef __INETWORKEVENTMANAGER_H__
#define __INETWORKEVENTMANAGER_H__

class ISockObj;

class INetworkEventManager
{
public:
    enum EEvent
    {
        EEVENT_MIN,
        EEVENT_ACCEPT = EEVENT_MIN,
        EEVENT_CONNECT,
        EEVENT_RECV,
        EEVENT_CLOSE,
        EEVENT_CREATEFAIL,
        EEVENT_BINDFAIL,
        EEVENT_LISTENFAIL,
        EEVENT_POSTACCEPTFAIL,
        EEVENT_POSTCONNECTFAIL,
        EEVENT_POSTSENDFAIL,
        EEVENT_POSTRECVTFAIL,
        EEVENT_MAX,
    };
    struct Event
    {
        ISockObj*   pSockObj;
        EEvent      eEvent;
        ISockObj*   pAccept;
        Packet*     pRecv;
        int         iRetCode;
    };
    virtual int HandleEvent(Event event) = 0;
};

#endif // __INETWORKEVENTMANAGER_H__