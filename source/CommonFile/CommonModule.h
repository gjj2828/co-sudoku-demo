#ifndef __COMMONMODULE_H__
#define __COMMONMODULE_H__

GlobalEnviroment* gEnv = NULL;

void ModuleInit(GlobalEnviroment* env)
{
    gEnv = env;
}

#endif // __COMMONMODULE_H__