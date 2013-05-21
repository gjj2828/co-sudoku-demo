//**************************************************
//File: main.cpp
//Author: GaoJiongjiong
//Function: äÖÈ¾Ä£¿é
//**************************************************

#include "StdAfx.h"
#include "RenderSystem.h"

extern "C"
{
    DLL_EXPORT IRenderSystem* CreateRenderSystem()
    {
        static char buffer[sizeof(CRenderSystem)];
        return new ((void*)buffer) CRenderSystem();
    }
};