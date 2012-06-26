#ifndef __GAME_H__
#define __GAME_H__

#include <IGame.h>

class CGame: public IGame
{
public:
    virtual bool    Init()      {return true;}
    virtual void    Run()       {printf("CGame::Run");}
    virtual void    Release()   {this->~CGame();}
};

#endif // __GAME_H__

