#ifndef __INETWORKSYSTEM_H__
#define __INETWORKSYSTEM_H__

class INetworkListener
{
public:
    virtual void OnPair() {}
    virtual void OnDisconnect() {}
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
    virtual void    Update(float time)                              = 0;
    virtual int     Start(EMode mode, float time)                   = 0;
    virtual void    Stop()                                          = 0;
    virtual void    RegisterListener(INetworkListener* listener)    = 0;
    virtual void    UnRegisterListener(INetworkListener* listener)  = 0;
};

#endif // __INETWORKSYSTEM_H__