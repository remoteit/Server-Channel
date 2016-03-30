#ifndef __CONFIG_H__
#define __CONFIG_H__
/*! \file config.h
    \brief Configuation file for Yoics chat client.
*/
#include "mytypes.h"

#if !defined(WINCE)
#include <sys/types.h>
#endif

#include <stdio.h>

#define GETHOSTBYNAME 1

#define	VERSION		"0.2"
#define VER			0
#define VER_SUB	    2


#if defined(ARM) && !defined(WINCE)
// Align on 4 byte boundry for ARM
#define ALIGN4			__attribute__ ((aligned (4)));	
#else
#define ALIGN4			;
#endif



#if defined(ANDROID)

//#define stdin 0
//#define stdout 1
//#define stderr 2

#endif

#if !defined(__ECOS)

//#define ALLOC_DEBUG 1

#endif


#if defined(PINETRON)
#define __stdin stdin
#define __stdout stdout
#endif


#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif



// pack all structures to byte level, this should work for all compilers
#pragma pack(1)

//******************************************************************************
// For windows include the following
//******************************************************************************
#if defined(WIN32)

#ifndef TRACE
#define TRACEIN
#define TRACEOUT
#define TRACE
#endif

#ifndef _MT 
#define _MT 
#endif 

#ifndef WINDOWS
#define WINDOWS
#endif 

// Disable warnings for strcpy,strcat
//#define _CRT_SECURE_NO_WARNINGS // Disable deprecation warning in VS2005

// Disable WIN32_LEAN_AND_MEAN.
// This makes windows.h always include winsock2.h
#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif

//#define LITTLE_ENDIAN
//#ifndef  WIN32_LEAN_AND_MEAN 
//#include <winsock.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>    /* _beginthread, _endthread */
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include <time.h>
#include <signal.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <Rpc.h>
#include <string.h>
#include <assert.h>
#include <process.h>

#if defined(WIN_UPNP_SUPPORT)
#include <Natupnp.h>
#endif

#define __func__ __FUNCTION__

// Define syslog as null on windows
#define  syslog	
#define	 openlog

#define  LOG_ERR        3
#define  LOG_WARNING	4
#define	 LOG_INFO		6

#define strtok_r	strtok_s
#define getpid		_getpid
#define kbhit		_kbhit
#define snprintf	_snprintf
#define strcasecmp	_stricmp
#define unlink      _unlink

#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>

// Winsock fix
#define MSG_WAITALL 0x8 

//#define FD_SETSIZE 512

// Unix some winsock errors
#define EWOULDBLOCK		WSAEWOULDBLOCK
#define EINPROGRESS		WSAEINPROGRESS
#define EALREADY		WSAEALREADY
#define EINVAL			WSAEINVAL
#define EISCONN			WSAEISCONN
#define ENOTCONN		WSAENOTCONN

#define usleep			Sleep



//#endif
#endif


//******************************************************************************
// For linux/unix
//******************************************************************************
#if defined(LINUX) || defined(MACOSX) || defined(IOS)
// For Linux include the following
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <assert.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <net/if.h>
#include <signal.h>
#include <errno.h>
#include <sys/timeb.h>
#include <sched.h>
#include <syslog.h>
#include <ctype.h>
#include <assert.h>
#include <dirent.h>


#ifndef TRACE
#define TRACEIN
#define TRACEOUT
#define TRACE(...)
#endif


#if defined(MACOSX)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#if defined(BACKTRACE_SYMBOLS)
#include <execinfo.h>
#endif
#include <setjmp.h>

#define MAX_PATH PATH_MAX

#endif

#if defined(WINCE)
#define _tfunc AnsiFunc

#include <windows.h>
#include <winbase.h>
//#include <winsock.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Rpc.h>
#include <string.h>
#include <assert.h>

#define socklen_t int
#define strerror(n) _T("file error")

// Winsock fix
#define MSG_WAITALL 0x8 

// Unix some winsock errors
#define EWOULDBLOCK		WSAEWOULDBLOCK
#define EINPROGRESS		WSAEINPROGRESS
#define EALREADY		WSAEALREADY
#define EINVAL			WSAEINVAL
#define EISCONN			WSAEISCONN
#define ENOTCONN		WSAENOTCONN

#define usleep			Sleep
#define snprintf _snprintf


#define openlog()
#define syslog()

#define	close			closesocket

#undef WIN32

#endif

#if defined(__ECOS)

#include <network.h>
#include <pkgconf/net.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <prod_config.h>
#include <cyg/libc/time/time.h>
#include <time.h>

#endif

//#define DEV_KIT 1

#if defined(LINUX) || defined(MACOSX) || defined(__ECOS) || defined(IOS)

#define SOCKET_ERROR		-1
#define INVALID_SOCKET		-1
#define closesocket			close

#define TRUE				1 
#define FALSE				0

#endif




#endif


