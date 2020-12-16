#ifndef __DEBUG_H__
#define __DEBUG_H__

//---------------------------------------------------------------------------
// debug.h - Debug equates for messaging at different debug levels			-
//---------------------------------------------------------------------------
// Version                                                                  -
//		0.1 Original Version Jan 14, 2001									-
//																			-
// (c)2001 Mycal Labs, All Rights Reserved									-
//---------------------------------------------------------------------------

#if defined(WEB_PROXY) || defined(DROBO) || defined(TEST_FIX)
#define DEBUG_LV0	1                               /* server and p2p connect info */
#endif

//#define DEBUG_LV0	1	
//#define DEBUG_LV1	1								
//#define DEBUG_LV2	1
//#define DEBUG_LV3	1								/* Packet Monitoring (types) */
//#define DEBUG_LV4	1								/* memory debug */
//#define DEBUG_LV5	1								/*tunnel debug create destroy*/
//#define DEBUG_LV6	1								/*tunnel debug, rtt and tunnel ack*/
//#define DEBUG_LV7	1								/*special temp*/
//#define DEBUG_LV8	1								/*not used*/
//#define DEBUG_LV9	1								/*Misc temporary debug*/
//#define PACKET_RX_DEBUG 1

#if defined(WEB_PROXY)
#if DEBUG_LV1 || DEBUG_LV2 || DEBUG_LV3 || DEBUG_LV4 || DEBUG_LV5 || DEBUG_LV6 || DEBUG_LV7 || DEBUG_LV8 || DEBUG_LV9 
#define DEBUG_LV_ON 1
#endif
#else
#if DEBUG_LV0 || DEBUG_LV1 || DEBUG_LV2 || DEBUG_LV3 || DEBUG_LV4 || DEBUG_LV5 || DEBUG_LV6 || DEBUG_LV7 || DEBUG_LV8 || DEBUG_LV9 
#define DEBUG_LV_ON 1
#endif
#endif

//#if DEBUG_LV1 || DEBUG_LV2 123
//#include <stdio.h> 
//#endif


//
// Debug level 0 proxy application debug
//
#ifdef DEBUG_LV0
	#define	DEBUG0			yprintf
#else
#ifdef WIN32
    #define		DEBUG0(...)      { /* empty */ }
#else
	#define DEBUG0(...)		{ /* empty */ }
#endif
#endif
//
// Debug level 1 proxy application debug
//
#ifdef DEBUG_LV1
	#define	DEBUG1			yprintf
#else
#ifdef WIN32
#define		DEBUG1(...)          { /* empty */ }
#else
	#define DEBUG1(...)		//
#endif
#endif

#ifdef DEBUG_LV2
	#define	DEBUG2			yprintf
#else
#ifdef WIN32
#define		DEBUG2(...)          { /* empty */ }
#else
	#define DEBUG2(...)		//
#endif
#endif

#ifdef DEBUG_LV3
	#define	DEBUG3			yprintf
#else
#ifdef WIN32
#define		DEBUG3(...)          { /* empty */ }
#else
	#define DEBUG3(...)		//
#endif	
#endif

#ifdef DEBUG_LV4
	#define	DEBUG4			yprintf
#else
#ifdef WIN32
#define		DEBUG4(...)          { /* empty */ }
#else
	#define DEBUG4(...)		//
#endif	
#endif

// Debug level 5 is for RTT debugging
#ifdef DEBUG_LV5
	#define	DEBUG5			yprintf
#else
#ifdef WIN32
#define		DEBUG5(...)          { /* empty */ }
#else
	#define DEBUG5(...)		//
#endif		//				//
#endif

// Debug level 6 is for RTT debugging
#ifdef DEBUG_LV6
	#define	DEBUG6			yprintf
#else
#ifdef WIN32
#define		DEBUG6(...)          { /* empty */ }
#else
	#define DEBUG6(...)		//
#endif		//				//
#endif

// Debug level 6 is for RTT debugging
#ifdef DEBUG_LV7
	#define	DEBUG7			yprintf
#else
#ifdef WIN32
#define		DEBUG7(...)          { /* empty */ }
#else
	#define DEBUG7(...)		//
#endif		//				//
#endif

// Debug level 8 i
#ifdef DEBUG_LV8
#define	DEBUG8			yprintf
#else
#ifdef WIN32
#define		DEBUG8(...)          { /* empty */ }
#else
#define DEBUG8(...)		//
#endif		//				//
#endif

// Debug level 9 is for testing specific things and is not set to one area.
#ifdef DEBUG_LV9
	#define	DEBUG9			yprintf
#else
#ifdef WIN32
#define		DEBUG9(...)          { /* empty */ }
#else
	#define DEBUG9(...)		//
#endif		//				//
#endif


#endif
