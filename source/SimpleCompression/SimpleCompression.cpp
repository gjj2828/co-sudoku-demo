#include "StdAfx.h"
#include "SimpleCompression.h"

EEncodeRet  CSimpleCompression::Encode(const ComPuzExpr& puzzle, UCHAR* buffer, int& length)
{
    if(length < LENGTH)
    {
        length = LENGTH;
        return ENCODERET_BUFFERLACK;
    }
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
            a   = puzzle.data[index] * b;
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