#ifndef __SIMPLECOMPRESSION_H__
#define __SIMPLECOMPRESSION_H__

#include <ICompression.h>

class CSimpleCompression: public ICompression
{
public:
    virtual EEncodeRet  Encode(const ComPuzExpr& puzzle, UCHAR* buffer, int& length)    {return ENCODERET_SUCCESS;}
    virtual EDecodeRet  Decode(ComPuzExpr& puzzle, UCHAR* buffer, int length)           {return EDECODERET_SUCCESS;}
};

#endif // __SIMPLECOMPRESSION_H__