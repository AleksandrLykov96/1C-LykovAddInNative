// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef __STDAFX_H__
#define __STDAFX_H__

#define NDEBUG

#ifndef __linux__
	#if ENABLE_MY_SQL
		#include <WinSock2.h>
	#endif

	#include <windows.h>
#endif //__linux__

#include <baseClass/globalFunctions.hpp>

#endif //__STDAFX_H__
