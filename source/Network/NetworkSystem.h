#ifndef __NETWORKSYSTEM_H__
#define __NETWORKSYSTEM_H__

#include <INetworkSystem.h>

class CNetworkSystem: public INetworkSystem
{
public:
    CNetworkSystem();
    virtual int     Init();
    virtual void    Release();
    virtual void    Update();
    virtual void    Start(EMode mode);
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

    EState m_eState;
};

#endif // __NETWORKSYSTEM_H__