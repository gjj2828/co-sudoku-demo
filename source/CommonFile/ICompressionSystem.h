#ifndef __ICOMPRESSOINSYSTEM_H__
#define __ICOMPRESSOINSYSTEM_H__

enum ECompressorType
{
    ECOMPRESSORTYPE_MIN,
    ECOMPRESSORTYPE_SIMPLE  = ECOMPRESSORTYPE_MIN,
    ECOMPRESSORTYPE_MAX,
};

enum EEncodeRet
{
    ENCODERET_MIN,
    ENCODERET_SUCCESS  = ENCODERET_MIN,
    ENCODERET_TYPEERR,
    ENCODERET_BUFFERLACK,
    ENCODERET_UNKNOWERR,
    ENCODERET_MAX,
};

enum EDecodeRet
{
    EDECODERET_MIN,
    EDECODERET_SUCCESS  = EDECODERET_MIN,
    EDECODERET_TYPEERR,
    EDECODERET_BUFFERLACK,
    EDECODERET_UNKNOWERR,
    EDECODERET_MAX,
};

class ICompressionSystem
{
public:
    EEncodeRet  Encode(ECompressorType type, const ComPuzExpr& puzzle, UCHAR* buffer, int& length);
    EDecodeRet  Decode(ECompressorType type, ComPuzExpr& puzzle, UCHAR* buffer, int length);
};

#endif // __ICOMPRESSOINSYSTEM_H__