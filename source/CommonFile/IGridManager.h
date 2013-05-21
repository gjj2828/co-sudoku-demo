//**************************************************
//File: IGridManager.h
//Author: GaoJiongjiong
//Function: ���ӹ���ӿ�
//**************************************************

#ifndef __IGRIDMANAGER_H__
#define __IGRIDMANAGER_H__

class IGridManager
{
public:
    enum
    {
        INVALID_GRID = 0xffff,
    };
    virtual void GetGrid(POINT pos, int& grid, int& sgrid) = 0;
};

#endif // __IGRIDMANAGER_H__