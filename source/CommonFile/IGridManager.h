#ifndef __IGRIDMANAGER_H__
#define __IGRIDMANAGER_H__

class IGridManager
{
public:
    enum
    {
        INVALID_GRID = 0xffff,
    };
    virtual void SetPos(POINT pos) = 0;
    virtual int GetSelectedGrid() = 0;
};

#endif // __IGRIDMANAGER_H__