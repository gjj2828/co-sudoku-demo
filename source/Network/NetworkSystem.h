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

    EState m_eState;
    float m_fTimeBase;
};

#endif // __NETWORKSYSTEM_H__