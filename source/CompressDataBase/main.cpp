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

int Compress(const char* in_name, const char* out_name)
{
    FILE* in = NULL;
    FILE* out = NULL;
    in = fopen(in_name, "r");
    out = fopen(out_name, "wb");
    if(!in || !out)
    {
        SAFE_FCLOSE(in);
        SAFE_FCLOSE(out);
    }

    enum {BUFLEN = 256, ENLEN=34};
    char buffer[BUFLEN];
    UCHAR enbuffer[ENLEN];
    CompressionHeader header;
    while(fgets(buffer, BUFLEN, in))
    {
        ComPuzExpr puzzle;
        FILLPUZZLE(puzzle, buffer);
        int enlen = ENLEN;
        g_pCompressions[ECOMPRESSION_SIMPLE]->Encode(puzzle, enbuffer, enlen);
        header.type = ECOMPRESSION_SIMPLE;
        header.length = enlen;
        fwrite(&header, sizeof(CompressionHeader), 1, out);
        fwrite(enbuffer, sizeof(UCHAR), ENLEN, out);
    }

    return 1;
}

void main()
{
    InitRootDir();

    if(!Init()) goto END;

    char InName[256];
    char OutName[256];
    do 
    {
        char* p= strrchr(g_FindData.name, '.');
        if(!p)  goto END;
        *p = 0;
        sprintf(InName, "DataBase\\Original\\%s.txt", g_FindData.name);
        sprintf(OutName, "DataBase\\%s.db", g_FindData.name);
        if(!Compress(InName, OutName)) goto END;
    }while(_findnext(g_FindHandle, &g_FindData) == 0);
END:
    Release();
}