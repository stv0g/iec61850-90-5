/****************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *********************************************/
/****************************************************************************/
/* (c) Copyright 2012 Systems Integration Specialists Company, Inc. (SISCO) */
/* 6605 19 1/2 Mile Road, Sterling Heights, Michigan, 48314 USA		    */
/* Tel: +1-586-254-0020   http://www.sisconet.com			    */
/*									    */
/* This Work consists of voluntary contributions made by SISCO and 	    */
/* individuals on behalf of SISCO and then contributed to the 		    */
/* community in support of furthering market acceptance of 		    */
/* IEC TR 61850-90-5. Please support the community by submitting 	    */
/* your modifications and contributions at:				    */
/*									    */
/* http://iec61850.ucaiug.org/90-5/default.aspx				    */
/* 									    */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may  */
/* not use this Work except in compliance with the License.		    */
/*									    */
/* You may obtain a copy of the License at				    */
/*									    */
/* http://www.apache.org/licenses/LICENSE-2.0				    */
/*									    */
/* A copy of the License is included in the distribution of this Work	    */
/* in the file named “90-5_License.txt”.				    */
/*									    */
/* Unless required by applicable law or agreed to in writing, software 	    */
/* distributed under the License is distributed on an "AS IS" BASIS,	    */
/* WITHOUT, WARRANTIES OR CONDITIONS OF ANY KIND, either express or 	    */
/* implied. See the License for the specific language governing  	    */
/* permissions andlimitations under the License.			    */
/*									    */
/*									    */
/* MODULE NAME : iec_sysincs.h						    */
/*									    */
/* MODULE DESCRIPTION : 						    */
/*	The purpose of this include file is to bring in include files	    */
/*	that come with one of the various C compilers.			    */
/*									    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who   Rev			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 10/04/11  HSF	   Initial revision				    */
/****************************************************************************/

#ifndef IECSYSINCS_INCLUDED
#define IECSYSINCS_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


#if defined (_WIN32)
#if !defined (_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE
#endif
#if !defined(__GNUC__)	/* GNUC doesn't support "#pragma warning"	*/
#pragma warning(disable : 4996)
#pragma warning(disable : 4786 4800)
#endif

#if defined (_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES)
#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#endif

/* winsock2.h MUST ALWAYS BE BEFORE windows.h to override defs in	*/
/* winsock.h (included by windows.h). Any module that includes windows.h*/
/* before sysincs.h, must also include winsock2.h before windows.h.	*/
#include <winsock2.h>		/* must be before windows.h	*/
#include <windows.h>
#include <process.h>		/* for _beginthread, _endthread	*/
#include <sys/timeb.h>		/* for ftime, timeb		*/
#endif  /* defined(_WIN32) */

#if defined(_WIN32) || defined(MSDOS) || defined(__MSDOS__)
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <conio.h>
#endif

#if defined(VXWORKS)
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <selectLib.h>
#include <limits.h>
#include <signal.h>		/* for "kill", etc.		*/
/* Sockets related includes	*/
#include <sys/socket.h>
#include <ioLib.h>
#include <sockLib.h>
#include <pipeDrv.h>
#include <sysLib.h>
#include <usrLib.h>
#include <netinet/in.h>		/* IPPROTO_*, etc.			*/
#include <arpa/inet.h>		/* inet_addr, etc.			*/
#include <netinet/tcp.h>	/* TCP_NODELAY, etc.			*/
#include <dirent.h>
#endif



/* UNIX or "UNIX-like" systems	*/
#if defined(_AIX) || defined(sun) || defined(__hpux) || defined(linux) \
    || (defined(__alpha) && !defined(__VMS)) || defined(__LYNX)
#include <unistd.h>	/* SHOULD be before any other include files 	*/
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#if (!defined(__LYNX))
#include <sys/time.h>
#include <sys/resource.h>
#endif
#define	max(a,b)	(((a) > (b)) ? (a) : (b))
#define	min(a,b)	(((a) < (b)) ? (a) : (b))
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <fcntl.h>		/* open, O_RDONLY, O_WRONLY, etc.	*/
#include <ctype.h>
#include <limits.h>

#if defined(linux)
#include <sys/timeb.h>		/* for ftime, timeb		*/
#include <dirent.h>		/* for POSIX directory functions*/
#endif

#if defined(__hpux)
#include <sys/termios.h>
#else
#include <termio.h>
#endif

#if (!defined (__hpux)) && (!defined(__LYNX))
#include <sys/select.h>
#endif
#include <signal.h>		/* for "kill", etc.		*/
#include <sys/ioctl.h>
#if defined(sun)
#include <sys/filio.h>
#endif
/* Sockets related includes	*/
#if defined(__LYNX)
#include <socket.h>
#else
#include <sys/socket.h>
#endif
#include <netdb.h>		/* gethostbyname, etc.			*/
#include <netinet/in.h>		/* IPPROTO_*, etc.			*/
#include <sys/un.h>		/* for sockaddr_un			*/
/* Forward references are supplied to eliminate xlC_r compiler warnings	*/
struct ether_addr;		/* forward reference			*/
struct sockaddr_dl;		/* forward reference			*/
#include <arpa/inet.h>		/* inet_addr, etc.			*/
#include <netinet/tcp.h>	/* TCP_NODELAY, etc.			*/
#ifndef INADDR_NONE
#define INADDR_NONE   ((in_addr_t) 0xffffffff)
#endif
#endif /* defined(_AIX) || defined(sun) || defined(__hpux) || defined(linux) ... */



#if (!defined(INT_MAX)) || (!defined(LONG_MAX))
#error INT_MAX and LONG_MAX must be defined. Usually defined in limits.h
#endif

#if defined(MAX_PATH)
#define S_MAX_PATH   MAX_PATH
#elif defined(PATH_MAX)    /* POSIX should have it defined in limits.h    */
#define S_MAX_PATH    PATH_MAX
#else
#define S_MAX_PATH    1024   /* default   */
#endif

	/*----------------------------------------------*/
	/* 	printf, sprintf, sscanf helper macros	*/
	/*----------------------------------------------*/

/* helper macro for 32-bit and 64-bit pointers support			*/
/* If pointer "0x%p" format is not supported then something like "0x%x",*/
/* "0x%lx", or "0x%llx" may be used, depending on the pointer size.    */
#if defined(_WIN32)
  #define S_FMT_PTR             "0x%p"
#elif defined(linux)
  /* "%p" format on linux produces output including "0x" prefix.	*/
  /* Set minimum length to 10, so column of pointers lines up.		*/
  #define S_FMT_PTR		"%10p"
#else  /* all other systems (e.g. UNIX)	*/
  #define S_FMT_PTR             "0x%p"
#endif /* all other systems (e.g. UNIX)	*/

/* helper macro for time_t	*/
#if defined(_WIN32)
  #if defined(_USE_32BIT_TIME_T)
    #define S_FMT_TIME_T        "%d"
  #else
    #define S_FMT_TIME_T        "%I64d"
  #endif
#else  /* all other systems (e.g. UNIX)	*/
  #define S_FMT_TIME_T          "%ld"
#endif /* all other systems (e.g. UNIX)	*/

#ifdef INT64_SUPPORT
#ifdef _WIN32
  #define S_FMT_INT64           "%I64d"
  #define S_FMT_UINT64          "%I64u"
  #define S_FMT_UINT64x         "0x%I64x"
  #define S_FMT_UINT64X         "0x%I64X"
#elif defined(_AIX) || defined(__hpux) || defined(linux) || defined(sun) || defined(__LYNX)
  #define S_FMT_INT64           "%lld"
  #define S_FMT_UINT64          "%llu"
  #define S_FMT_UINT64x         "0x%llx"
  #define S_FMT_UINT64X         "0x%llX"
#elif (defined(__alpha) && !defined(__VMS)) 
  #define S_FMT_INT64           "%ld"
  #define S_FMT_UINT64          "%lu"
  #define S_FMT_UINT64x         "0x%lx"
  #define S_FMT_UINT64X         "0x%lX"
#else  /* all other systems */
  #error Missing S_FMT_INT64 and S_FMT_UINT64 defines for this platform.
#endif /* all other systems */
#endif /* INT64_SUPPORT */

/* helper macro for HANDLE	*/
#if defined(_WIN32)
  #if (_MSC_VER >= 1300)
  #define S_FMT_HANDLE          "0x%p"
  #define S_FMT_THREAD_HANDLE   "0x%p"
  #else
  #define S_FMT_HANDLE          "%d"
  #define S_FMT_THREAD_HANDLE   "%lu"
  #endif
#else  /* all other systems (e.g. UNIX)	*/
  #define S_FMT_THREAD_HANDLE   "0x%p"
#endif /* all other systems (e.g. UNIX)	*/


/************************************************************************/
/************************************************************************/
/*		Assert stuff						*/
/************************************************************************/
/************************************************************************/

#include <assert.h>

#if defined (_WIN32) && defined (_DEBUG)
#include <crtdbg.h>
#endif



#if !defined (VERIFY)
#if !defined (NDEBUG)
#define VERIFY(e)	_SASSERTE(e)
#else
#define VERIFY(e)	((void) (e))
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif


