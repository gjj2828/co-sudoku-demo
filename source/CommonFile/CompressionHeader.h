//**************************************************
//File: CompressionHeader.h
//Author: GaoJiongjiong
//Function: 关卡压缩头文件
//**************************************************

#ifndef __COMPRESSIONHEADER_H__
#define __COMPRESSIONHEADER_H__

struct CompressionHeader
{
    UCHAR type      :2;
    UCHAR length    :6;
};

#endif // __COMPRESSIONHEADER_H__