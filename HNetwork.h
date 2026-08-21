#pragma once

#ifndef HNETWORK_FULL_SOURCE
    #ifndef HNETWORK_LIB_PATH
        //■=============================================================================================■
        //   라이브러리 기본 경로 세팅.
        //■=============================================================================================■
        #define HNETWORK_LIB_PATH "HNetwork/Lib"
    #endif

	//■=============================================================================================■
	//   라이브러리 이름 완성.
	//■=============================================================================================■
	#ifdef _WIN64
		#ifdef _DEBUG
			#define HNETWORK_LIB(PATH, VS)  PATH##"/HNetwork_"##VS##"_64_debug"##".lib"
		#else
			#define HNETWORK_LIB(PATH, VS)  PATH##"/HNetwork_"##VS##"_64.lib"
		#endif
	#else
		#ifdef _DEBUG
			#define HNETWORK_LIB(PATH, VS)  PATH##"/HNetwork_"##VS##"_32_debug"##".lib"
		#else
			#define HNETWORK_LIB(PATH, VS)  PATH##"/HNetwork_"##VS##"_32.lib"
		#endif
	#endif

	//■=============================================================================================■
	//   라이브러리 링크.
	//    - visual studio 2010 ~ 2019 지원.
	//■=============================================================================================■
    #if defined(_MSC_VER)
		#if (_MSC_VER >= 1920)
			#pragma comment(lib, HNETWORK_LIB(HNETWORK_LIB_PATH, "vs2019"))
		#elif (_MSC_VER >= 1910)
            #pragma comment(lib, HNETWORK_LIB(HNETWORK_LIB_PATH, "vs2017"))
        #elif (_MSC_VER >= 1900)
            #pragma comment(lib, HNETWORK_LIB(HNETWORK_LIB_PATH, "vs2015"))
        #elif (_MSC_VER >= 1800)
            #pragma comment(lib, HNETWORK_LIB(HNETWORK_LIB_PATH, "vs2013"))
        #elif (_MSC_VER >= 1700)
            #pragma comment(lib, HNETWORK_LIB(HNETWORK_LIB_PATH, "vs2012"))
        #endif
    #endif
#endif


#ifndef _WINSOCKAPI_ 
    #define _WINSOCKAPI_
#endif

#include "HNetCommon.h"
#include "HNetAcceptor.h"
#include "HNetConnector.h"
#ifdef HNETWORK_FULL_SOURCE
    #include "HNetWsAcceptor.h"
#endif


