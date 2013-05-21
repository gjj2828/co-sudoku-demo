//**************************************************
//File: GlobalEnviroment.h
//Author: GaoJiongjiong
//Function: 全局环境变量
//**************************************************

#ifndef __GLOBALENVIROMENT_H__
#define __GLOBALENVIROMENT_H__

class IGame;
class IPuzzleSystem;
class IRenderSystem;
class INetworkSystem;

struct GlobalEnviroment
{
    IGame*          pGame;
    IPuzzleSystem*  pPuzzleSystem;
    IRenderSystem*  pRenderSystem;
    INetworkSystem* pNetworkSystem;
};

extern GlobalEnviroment* gEnv;

#endif // __GLOBALENVIROMENT_H__