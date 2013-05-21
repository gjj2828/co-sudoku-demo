//**************************************************
//File: INetworkSystem.h
//Author: GaoJiongjiong
//Function: ÍøÂçÄ£¿é½Ó¿Ú
//**************************************************

#ifndef __INETWORKSYSTEM_H__
#define __INETWORKSYSTEM_H__

#include <Packet.h>

class INetworkListener
{
public:
    virtual void OnAccept(int client)       {}
    virtual void OnConnect()                {}
    virtual void OnRecv(Packet* pPacket)    {}
    virtual void OnDisconnect()             {}
    virtual void OnStop()                   {}
};

class INetworkSystem
{
public:
    enum EMode
    {
        EMODE_MIN,
        EMODE_AUTOPAIR = EMODE_MIN,
        EMODE_MAX,
    };

    virtual int     Init()                                          = 0;
    virtual void    Release()                                       = 0;
    virtual int     Start(EMode mode, float time)                   = 0;
    virtual void    Stop()                                          = 0;
    virtual int     Update(float time)                              = 0;
    virtual void    Send(Packet* packet)                            = 0;
    virtual void    Send(Packet* packet, int client)                = 0;
    virtual void    RegisterListener(INetworkListener* listener)    = 0;
    virtual void    UnRegisterListener(INetworkListener* listener)  = 0;
};

#endif // __INETWORKSYSTEM_H__