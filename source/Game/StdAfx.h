#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <CommonHeader.h>

#ifdef GAMEDLL_EXPORT
#define GAME_API    DLL_EXPORT
#else
#define GAME_API
#endif // GAMEDLL_EXPORT

#endif // __STDAFX_H__