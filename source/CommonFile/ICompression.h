#ifndef __ICOMPRESSION_H__
#define __ICOMPRESSION_H__

enum EEncodeRet
{
    ENCODERET_MIN,
    ENCODERET_SUCCESS  = ENCODERET_MIN,
    ENCODERET_BUFFERLACK,
    ENCODERET_UNKNOWNERR,
    ENCODERET_MAX,
};

enum EDecodeRet
{
    EDECODERET_MIN,
    EDECODERET_SUCCESS  = EDECODERET_MIN,
    EDECODERET_BUFFERLACK,
    EDECODERET_UNKNOWNERR,
    EDECODERET_MAX,
};

class ICompression
{
public:
    virtual EEncodeRet  Encode(const ComPuzExpr& puzzle, UCHAR* buffer, int& length)    = 0;
    virtual EDecodeRet  Decode(ComPuzExpr& puzzle, UCHAR* buffer, int length)           = 0;
};

#endif // __ICOMPRESSION_H__