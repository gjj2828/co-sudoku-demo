//**************************************************
//File: IPuzzleSystem.h
//Author: GaoJiongjiong
//Function: �ؿ�ģ��ӿ�
//**************************************************

#ifndef __IPUZZLESYSTEM_H__
#define __IPUZZLESYSTEM_H__

class IPuzzleSystem
{
public:
    virtual int     Init()      = 0;
    virtual void    Release()   = 0;
};

#endif // __IPUZZLESYSTEM_H__