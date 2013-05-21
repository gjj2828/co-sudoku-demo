//**************************************************
//File: main.cpp
//Author: GaoJiongjiong
//Function: ¼òµ¥µÄ¹Ø¿¨Ñ¹Ëõ
//**************************************************

#include "StdAfx.h"
#include "SimpleCompression.h"

extern "C"
{
    DLL_EXPORT ICompression* GetCompression()
    {
        static CSimpleCompression compression;
        return &compression;
    }
};