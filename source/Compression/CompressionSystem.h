#ifndef __COMPRESSIONSYSTEM_H__
#define __COMPRESSIONSYSTEM_H__

#include <ICompressionSystem.h>

class CCompressionSystem: public ICompressionSystem
{
public:
    virtual EEncodeRet  Encode(ECompressorType type, const ComPuzExpr& puzzle, UCHAR* buffer, int& length)  {return ENCODERET_SUCCESS;}
    virtual EDecodeRet  Decode(ECompressorType type, ComPuzExpr& puzzle, UCHAR* buffer, int length)         {return EDECODERET_SUCCESS;}
};

#endif // __COMPRESSIONSYSTEM_H__