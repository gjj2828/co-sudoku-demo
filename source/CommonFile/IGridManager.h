//**************************************************
//File: IGridManager.h
//Author: GaoJiongjiong
//Function: 格子管理接口
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
    virtual int GetGrid(POINT pos) = 0;
};

#endif // __IGRIDMANAGER_H__