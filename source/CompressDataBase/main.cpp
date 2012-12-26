#include "StdAfx.h"

typedef ICompression* (*CreateCompression)();

HMODULE         g_hModules[ECOMPRESSION_MAX];
ICompression*   g_pCompressions[ECOMPRESSION_MAX];
char*           g_ModuleName[ECOMPRESSION_MAX] =
{
    "SimpleCompression.dll",
};

intptr_t    g_FindHandle = -1;
_finddata_t g_FindData;

int InitModules()
{
    for(int i = ECOMPRESSION_MIN; i < ECOMPRESSION_MAX; i++)
    {
        g_hModules[i]       = NULL;
        g_pCompressions[i]  =  NULL;
    }
    for(int i = ECOMPRESSION_MIN; i < ECOMPRESSION_MAX; i++)
    {
        g_hModules[i] = LoadLibrary(g_ModuleName[i]);
        if(!g_hModules[i]) ERROR_RTN0("Can\'t load compression dll!");
        CreateCompression fCreateCompressionFunc = (CreateCompression)GetProcAddress(g_hModules[i], "GetCompression");
        if(!fCreateCompressionFunc) ERROR_RTN0("Can\' get GetCompression function!");
        g_pCompressions[i] = fCreateCompressionFunc();
        if(!g_pCompressions[i]) ERROR_RTN0("GetCompression failed!")
    }
    return 1;
}

int InitFind()
{
    char Spec[] = "DataBase\\Original\\*.txt";
    g_FindHandle = _findfirst(Spec, &g_FindData);
    if(g_FindHandle == -1) ERROR_RTN0("Can\'t init find handle!");
    return 1;
}

int Init()
{
    if(!InitModules())  return 0;
    if(!InitFind())  return 0;
    return 1;
}

void Release()
{
    for(int i = ECOMPRESSION_MIN; i < ECOMPRESSION_MAX; i++)
    {
        SAFE_FREELIBRARY(g_hModules[i]);
    }
    SAFE_FINDCLOSE(g_FindHandle);
}

int Compress(const char* in_name, const char* out_name, const char* def_name)
{
    FILE* in = NULL;
    FILE* out = NULL;
    FILE* def = NULL;
    if(fopen_s(&in, in_name, "r")
    || fopen_s(&out, out_name, "wb")
    || fopen_s(&def, def_name, "wb"))
    {
        SAFE_FCLOSE(in);
        SAFE_FCLOSE(out);
        SAFE_FCLOSE(def);
        return 0;
    }

    enum {BUFLEN = 256, ENLEN=34};
    char buffer[BUFLEN];
    UCHAR enbuffer[ENLEN];
    CompressionHeader header;
    int count = 0;
    while(fgets(buffer, BUFLEN, in))
    {
        ComPuzExpr puzzle;
        FILLPUZZLE(puzzle, buffer);
        int enlen = ENLEN;
        g_pCompressions[ECOMPRESSION_SIMPLE]->Encode(puzzle, enbuffer, enlen);
        header.type = ECOMPRESSION_SIMPLE;
        header.length = enlen;
        fwrite(enbuffer, sizeof(UCHAR), ENLEN, out);
        fwrite(&header, sizeof(CompressionHeader), 1, def);
        count++;
        printf("\rCompress %s:%d", in_name, count);
    }

    SAFE_FCLOSE(in);
    SAFE_FCLOSE(out);
    SAFE_FCLOSE(def);

    return 1;
}

void main()
{
    InitRootDir();

    if(!Init()) goto END;

    char InName[256];
    char OutName[256];
    char DefName[256];
    do 
    {
        char* p= strrchr(g_FindData.name, '.');
        if(!p)  goto END;
        *p = 0;
        sprintf_s(InName, 256, "DataBase\\Original\\%s.txt", g_FindData.name);
        sprintf_s(OutName, 256, "DataBase\\%s.db", g_FindData.name);
        sprintf_s(DefName, 256, "DataBase\\%s.def", g_FindData.name);
        if(!Compress(InName, OutName, DefName)) goto END;
        printf("\n");
    }while(_findnext(g_FindHandle, &g_FindData) == 0);
END:
    Release();
}