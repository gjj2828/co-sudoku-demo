//**************************************************
//File: IDownloadDatabase.cpp
//Author: GaoJiongjiong
//Function: 下载关卡数据库
//**************************************************

#ifndef __IDOWNLOADDATABASE_H__
#define __IDOWNLOADDATABASE_H__

class IDownloadDatabase
{
public:
    virtual int Init() = 0;
    virtual int Run() = 0;
};

#endif // __IDOWNLOADDATABASE_H__