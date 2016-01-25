/*!															   www.weaved.com			
 *---------------------------------------------------------------------------
 *! \file  server_channel_packet.c
 *  \brief Server Channel Packet Engine
 *																			
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version January 20 2015							-        
 *
 *---------------------------------------------------------------------------    
 *                                                             				-
 * Copyright (C) 2015, Weaved Inc, www.weaved.com						    -
 *                                                                         	-
 * $Date: mwj 2015/01/20 20:35:55 $
 *
 *---------------------------------------------------------------------------
 *
 * Notes:
 *
 *
 *
*/

#include "mytypes.h"
#include "config.h"
#include "net.h"
#include "webio.h"
#include "arch.h"
#include "debug.h"
#include "log.h"
#include "yselect.h"
#include "daemonize.h"
#include "server_channel.h"
#include "server_channel_packet.h"

// Recieve a packet here, timeout is in seconds
int
server_channel_rx(SC_CONFIG *sc, int timeout)
{
    int                 ret;
    int                 slen;
	struct sockaddr_in	server;				
    char                pkt[SC_MAX_PACKET+1];

    // Set the timeout
    set_sock_recv_timeout(sc->udp_listen_soc,timeout);

    // Receive on UDP socket
    memset(&server,'\0',sizeof(struct sockaddr));
    slen=sizeof(struct sockaddr_in);
    ret=recvfrom(sc->udp_listen_soc, pkt, SC_MAX_PACKET, 0, (struct sockaddr *)&server, (socklen_t *) &slen);
    if(ret>0)
	{
        sc->rx_packets++;
        // We have datagram, lets process it
        pkt[SC_MAX_PACKET]=0;
        printf("we have a packet len %d ==>%s\n",ret,pkt);
        //---------------------------
        // Packet Handler Here      -
        //---------------------------



    }
	else if(SOCKET_ERROR==ret)
	{
        sc->rx_errors++;
		ret=get_last_error();
		if(ret!=EWOULDBLOCK)
			DEBUG3("Error %d on main read\n",ret);
	}

    return(-1);
}
