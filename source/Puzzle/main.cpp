#include "StdAfx.h"
#include "PuzzleSystem.h"

extern "C"
{
    DLL_EXPORT IPuzzleSystem* CreatePuzzleSystem()
    {
        static char buffer[sizeof(CPuzzleSystem)];
        return new ((void*)buffer) CPuzzleSystem();
    }
};