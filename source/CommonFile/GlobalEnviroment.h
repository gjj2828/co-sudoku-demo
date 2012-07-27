#ifndef __GLOBALENVIROMENT_H__
#define __GLOBALENVIROMENT_H__

class IGame;
class IPuzzleSystem;
class IRenderSystem;

struct GlobalEnviroment
{
    IGame*          pGame;
    IPuzzleSystem*  pPuzzleSystem;
    IRenderSystem*  pRenderSystem;
};

extern GlobalEnviroment* gEnv;

#endif // __GLOBALENVIROMENT_H__