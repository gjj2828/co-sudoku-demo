#ifndef __IRENDERSYSTEM_H__
#define __IRENDERSYSTEM_H__

class IRenderSystem
{
public:
    virtual int     Init()      = 0;
    virtual void    Release()   = 0;
};

#endif // __IRENDERSYSTEM_H__