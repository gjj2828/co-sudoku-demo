#ifndef __RENDERSYSTEM_H__
#define __RENDERSYSTEM_H__

#include <IRenderSystem.h>

class CRenderSystem: public IRenderSystem
{
public:
    virtual int     Init()      {return 1;}
    virtual void    Release()   {this->~CRenderSystem();}
};

#endif // __RENDERSYSTEM_H__