#include "StdAfx.h"
#include "SimpleCompression.h"

EEncodeRet  CSimpleCompression::Encode(const ComPuzExpr& puzzle, UCHAR* buffer, int& length)
{
    if(length < LENGTH)
    {
        length = LENGTH;
        return ENCODERET_BUFFERLACK;
    }
    ZeroMemory(buffer, sizeof(UCHAR) * length);
    std::bitset<BITLEN>    bit;
    for(int i = 0; i < ROWNUM; i++)
    {
        UINT    a   = 0;
        for(int j = 0; j < COLUMNNUM; j++)
        {
            UINT    b   = 1;
            for(int k = 0; k < j; k++)
            {
                b   *= 10;
            }
            int index   = i * COLUMNNUM + j;
            a += puzzle.data[index] * b;
        }
        for(int j = 0; j < ROWBITLEN; j++)
        {
            int index = i * ROWBITLEN + j;
            bit[index] = (a >> j) & 0x01;
        }
    }

    BITTOCHARBUFFER(bit, buffer, BITLEN, LENGTH);

    length = LENGTH;

    return ENCODERET_SUCCESS;
}

EDecodeRet  CSimpleCompression::Decode(ComPuzExpr& puzzle, UCHAR* buffer, int length)
{
    if(length < LENGTH) return EDECODERET_BUFFERLACK;

    std::bitset<BITLEN> bit;
    CHARBUFFERTOBIT(buffer, bit, LENGTH, BITLEN);

    for(int i = 0; i < ROWNUM; i++)
    {
        UINT a = 0;
        for(int j = 0; j < ROWBITLEN; j++)
        {
            int     index   = i * ROWBITLEN + j;
            UINT    b       = bit[index];
            a              |= (b << j);
        }
        for(int j = 0; j < COLUMNNUM; j++)
        {
            int index = i * COLUMNNUM + j;

            puzzle.data[index]  = (a % 10);
            a                  /= 10;
        }
    }

    return EDECODERET_SUCCESS;
}