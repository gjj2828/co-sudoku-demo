#ifndef __GAME_H__
#define __GAME_H__

#include <IGame.h>
#include <IPuzzleSystem.h>

class CGame: public IGame
{
public:
    CGame();
    virtual int     Init();
    virtual void    Run()       {}
    virtual void    Release();

private:
    enum EModule
    {
        EMODULE_MIN,
        EMODULE_PUZZLE  = EMODULE_MIN,
        EMODULE_MAX,
    };

    HMODULE         m_hModules[EMODULE_MAX];
    IPuzzleSystem*  m_pPuzzleSystem;
};

#endif // __GAME_H__

