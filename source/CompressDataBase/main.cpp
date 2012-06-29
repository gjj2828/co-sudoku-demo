#include "StdAfx.h"

void    main()
{
    InitRootDir();

    char        Spec[] = "DataBase\\Original\\*.txt";
    _finddata_t FindData;
    intptr_t    Handle = _findfirst(Spec, &FindData);
    if(Handle == -1)    return;
    char InName[256];
    char OutName[256];
    FILE* in = NULL;
    FILE* out = NULL;
    do 
    {
        sprintf(InName, "DataBase\\Original\\%s", FindData.name);
        char* p= strrchr(InName, '.');
        if(!p)  goto END;
        *p = 0;
        sprintf(OutName, "%s.db", InName);
        *p = '.';
        in = fopen(InName, "r");
        out = fopen(OutName, "w");
        if(!out || !in) goto END;
        SAFE_FCLOSE(in);
        SAFE_FCLOSE(out);
    }while(_findnext(Handle, &FindData) == 0);
END:
    SAFE_FCLOSE(in);
    SAFE_FCLOSE(out);
    _findclose(Handle);
}