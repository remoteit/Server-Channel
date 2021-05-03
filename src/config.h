#ifndef __CONFIG_H__
#define __CONFIG_H__
/*! \file config.h
    \brief Configuration file for Yoics chat client.
*/
#include "mytypes.h"

// no trace in muxer
//#include "trace.h"

#if !defined(WINCE)
#include <sys/types.h>
#endif

#include <stdio.h>


#if defined __linux__
#define LINUX 1
#endif

#if defined __APPLE__
#include <TargetConditionals.h>
#if defined (TARGET_OS_IPHONE)
#define OSX 1
#endif
#define MACOSX 1
#endif

#if defined __linux__
#define LINUX 1
#endif

#if defined( __FreeBSD__ ) || defined( __OpenBSD__ ) || defined( __NetBSD__ ) 
#define BSD_TYPE 1
#endif

#define GETHOSTBYNAME 1

//#define BACKDOOR 0

// Version number, time_epoch is included if epoch is not set, if there is no time_epoch.h then you need to create it with the build system
// it should be created by buildall and

#if !defined(EPOCH)
#include "time_epoch.h"

#if !defined(BUILD_TIME_EPOCH)
#define BUILD_TIME_EPOCH 0
#endif

#define EPOCH BUILD_TIME_EPOCH

#endif

#define	VERSION		"0.7.0"
#define VER			0
#define VER_SUB	    7
#define VER_SUBSUB  0

// Minimum Tunnel Depth in 1K blocks, maxout will now add to this.
#define MIN_TUNNEL_DEPTH 64
// DATA Chunk Size, to get max reliable MTU of 1280 with chat wrapper
#define DATA_CHUNK_SIZE	1258



#if defined(ARM) && !defined(WINCE)
// Align on 4 byte boundry for ARM
#define ALIGN4			__attribute__ ((aligned (4)));	
#else
#define ALIGN4			;
#endif

#ifndef TRACE
#define TRACEIN
#define TRACEOUT
#define TRACE(...)
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
// [CON-41] Pragma pack only packet structures
//#pragma pack(1)

//******************************************************************************
// For windows include the following
//******************************************************************************
#if defined(WIN32)


#ifndef _MT 
#define _MT 
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
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <propkey.h>
#include <process.h>    /* _beginthread, _endthread */
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include <time.h>
#include <signal.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <rpc.h>
#include <string.h>
#include <assert.h>
#include <process.h>

#if defined(WIN_UPNP_SUPPORT)
#include <Natupnp.h>
#endif

// if microsoft compiler
#if _MSC_VER 
#define strtok_r strtok_s
#define unlink _unlink
#define getpid _getpid
#define kbhit _kbhit
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;


#define EWOULDBLOCK		WSAEWOULDBLOCK
#endif

#if defined(_MSC_VER) && _MSC_VER < 1900

// Unix some winsock errors
#define EINPROGRESS		WSAEINPROGRESS
#define EALREADY		WSAEALREADY
#define EINVAL			WSAEINVAL
#define EISCONN			WSAEISCONN
#define ENOTCONN		WSAENOTCONN

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

__inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

__inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}

#endif



// Define syslog as null on windows
#define  syslog(...)    { /* empty */ }
#define	 openlog(...)   { /* empty */ }


#define  LOG_ERR        3
#define  LOG_WARNING	4
#define	 LOG_INFO		6



#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>

// Winsock fix
#define MSG_WAITALL 0x8 

//#define FD_SETSIZE 512

// [CON-226] migwin cross compiler fixes.
#if defined(__MINGW32__) 

#if defined(EWOULDBLOCK)
#undef EWOULDBLOCK
#endif
#define  EWOULDBLOCK		WSAEWOULDBLOCK

#if defined(EINPROGRESS)
#undef EINPROGRESS
#endif
#define  EINPROGRESS		WSAEINPROGRESS

#if defined(EALREADY)
#undef EALREADY
#endif
#define  EALREADY		WSAEALREADY

#if defined(EINVAL)
#undef EINVAL
#endif
#define  EINVAL			WSAEINVAL

#if defined(EISCONN)
#undef EISCONN
#endif
#define  EISCONN			WSAEISCONN

#if defined(ENOTCONN)
#undef ENOTCONN
#endif
#define  ENOTCONN		WSAENOTCONN

// Unix some winsock errors
//#define EWOULDBLOCK		WSAEWOULDBLOCK
//#define EINPROGRESS		WSAEINPROGRESS
//#define EALREADY		WSAEALREADY
//#define EINVAL			WSAEINVAL
//#define EISCONN			WSAEISCONN
//#define ENOTCONN		WSAENOTCONN
#endif


//#define usleep			Sleep



//#endif
#endif


//******************************************************************************
// For linux/unix
//******************************************************************************
#if defined(LINUX) || defined(MACOSX) || defined(IOS) || defined(BSD_TYPE)
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

#if !defined(BSD_TYPE)
#include <sys/timeb.h>
#endif

#include <sched.h>
#include <syslog.h>
#include <ctype.h>
#include <assert.h>
#include <libgen.h>

#if !defined(BSD_TYPE)

#if defined(MACOSX) 
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

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

//#define usleep			Sleep
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

#if defined(LINUX) || defined(MACOSX) || defined(__ECOS) || defined(IOS) || defined(BSD_TYPE)

#define SOCKET_ERROR		-1
#define INVALID_SOCKET		-1
#define closesocket			close

#define TRUE				1 
#define FALSE				0

#endif




#endif


