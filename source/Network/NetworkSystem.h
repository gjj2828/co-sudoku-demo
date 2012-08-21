#ifndef __NETWORKSYSTEM_H__
#define __NETWORKSYSTEM_H__

#include <INetworkSystem.h>

class CNetworkSystem: public INetworkSystem
{
public:
    CNetworkSystem();
    virtual int     Init();
    virtual void    Release();
    virtual void    Update(float time);
    virtual void    Start(EMode mode, float time);
    virtual void    Stop() {}
    virtual void    RegisterListener(INetworkListener* listener) {}
    virtual void    UnRegisterListener(INetworkListener* listener) {}

private:
    enum EState
    {
        ESTATE_MIN,
        ESTATE_STOPPED = ESTATE_MIN,
        ESTATE_WAITING,
        ESTATE_PAIRED,
        ESTATE_MAX,
    };

    enum
    {
        MAIN_PORT = 7778,
        BROADCAST_PORT = 7779,
    };

    EState m_eState;
    float m_fTimeBase;

    sockaddr_in m_BroadCastBindAddr;
    sockaddr_in m_BroadCastAddr;
    sockaddr_in m_BindAddr;

    SOCKET m_soBroadCast; 

    static const char* m_cBroadCastAddr;
};

#endif // __NETWORKSYSTEM_H__