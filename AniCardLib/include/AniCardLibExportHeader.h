#pragma once
#ifdef	ANICARDLIB_EXPORTS
#define ANICARDLIB_SHARED __declspec( dllexport )
#else
#define ANICARDLIB_SHARED __declspec( dllimport )
#endif