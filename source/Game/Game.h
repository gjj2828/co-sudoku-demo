#ifndef __GAME_H__
#define __GAME_H__

#include <IGame.h>
#include <IPuzzleSystem.h>
#include <IRenderSystem.h>

class CGame: public IGame
{
public:
    CGame();
    virtual int     Init(HINSTANCE hInstance);
    virtual void    Run();
    virtual void    Release();

private:
    enum EModule
    {
        EMODULE_MIN,
        EMODULE_PUZZLE  = EMODULE_MIN,
        EMODULE_RENDER,
        EMODULE_MAX,
    };

    HINSTANCE       m_hInstance;
    HWND            m_hWnd;
    HMODULE         m_hModules[EMODULE_MAX];
    IPuzzleSystem*  m_pPuzzleSystem;
    IRenderSystem*  m_pRenderSystem;

    int LoadDll();
    int InitWindow();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif // __GAME_H__

