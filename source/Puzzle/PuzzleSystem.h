//**************************************************
//File: PuzzleSystem.h
//Author: GaoJiongjiong
//Function: ¹Ø¿¨Ä£¿é
//**************************************************

#ifndef __PUZZLESYSTEM_H__
#define __PUZZLESYSTEM_H__

#include <IPuzzleSystem.h>

class CPuzzleSystem: public IPuzzleSystem
{
public:
    virtual int     Init()      {return 1;}
    virtual void    Release()   {this->~CPuzzleSystem();}

private:
    HMODULE hCompressionModules[ECOMPRESSION_MAX];
};

#endif // __PUZZLESYSTEM_H__