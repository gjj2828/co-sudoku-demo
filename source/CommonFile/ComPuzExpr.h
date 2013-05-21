//**************************************************
//File: ComPuzExpr.h
//Author: GaoJiongjiong
//Function: 关卡通用表达式
//**************************************************

#ifndef __COMPUZEXPR_H__
#define __COMPUZEXPR_H__

struct ComPuzExpr
{
    enum    {SIZE = 81};
    char    data[SIZE];
};

#define FILLPUZZLE(expr, buffer)                \
{                                               \
    for(int i = 0; i < ComPuzExpr::SIZE; i++)   \
    {                                           \
        (expr).data[i] = (buffer)[i] - '0';                \
    }                                           \
}

#endif // __COMPUZEXPR_H__