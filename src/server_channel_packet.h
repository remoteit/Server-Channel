#ifndef __SERVER_CHANNEL_PACKET_H__
#define __SERVER_CHANNEL_PACKET_H__
/*! \file server_channel_PACKET.h

*/
#include "config.h"
#include "mytypes.h"


#define SC_MAX_PACKET 512


/*! \fn int server_channel_rx(SC_CONFIG *sc, int timeout);
	
	\brief Receive packets from server channel

	\return -1 if timout was reached.
*/
int server_channel_rx(SC_CONFIG *sc, int timeout);


#define TEST_SERVER_CHANNEL_PACKET 1
#if defined(TEST_SERVER_CHANNEL_PACKET)
// For testing
int test_parse_line(void);
#endif

#endif

