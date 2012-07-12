#ifndef __GLOBALENVIROMENT_H__
#define __GLOBALENVIROMENT_H__

struct IGame;
struct IPuzzleSystem;
struct IRenderSystem;

struct GlobalEnviroment
{
    IGame*          pGame;
    IPuzzleSystem*  pPuzzleSystem;
    IRenderSystem*  pRenderSystem;
};

extern GlobalEnviroment* gEnv;

#endif // __GLOBALENVIROMENT_H__