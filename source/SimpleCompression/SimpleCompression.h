//**************************************************
//File: SimpleCompression.h
//Author: GaoJiongjiong
//Function: ¼òµ¥µÄ¹Ø¿¨Ñ¹Ëõ
//**************************************************

#ifndef __SIMPLECOMPRESSION_H__
#define __SIMPLECOMPRESSION_H__

#include <ICompression.h>

class CSimpleCompression: public ICompression
{
public:
    virtual EEncodeRet  Encode(const ComPuzExpr& puzzle, UCHAR* buffer, int& length);
    virtual EDecodeRet  Decode(ComPuzExpr& puzzle, UCHAR* buffer, int length);
private:
    enum
    {
        LENGTH      = 34,
        ROWNUM      = 9,
        COLUMNNUM   = ROWNUM,
        ROWBITLEN   = 30,
        BITLEN      = ROWBITLEN * ROWNUM,
    };
};

#endif // __SIMPLECOMPRESSION_H__