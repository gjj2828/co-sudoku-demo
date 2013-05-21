//**************************************************
//File: CommonModule.h
//Author: GaoJiongjiong
//Function: 模块间通用内容
//**************************************************

#ifndef __COMMONMODULE_H__
#define __COMMONMODULE_H__

GlobalEnviroment* gEnv = NULL;

void ModuleInit(GlobalEnviroment* env)
{
    gEnv = env;
}

#endif // __COMMONMODULE_H__