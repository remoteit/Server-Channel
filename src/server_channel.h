#ifndef __SERVER_CHANNEL_H__
#define __SERVER_CHANNEL_H__
/*! \file server_channel.h

*/
#include "config.h"
#include "mytypes.h"
#include "yhash.h"

//
// GF flags, global flags
//
#define	GF_GO			0x01				/* go */
#define GF_DAEMON		0x02				/* we are a daemon */
#define GF_QUIET		0x08				/* no output */
#define GF_CMD_LINE		0x10				/*  */
#define GF_CMD_PROC		0x20				/* Turn on command line processor */
#define GF_BANG_STATUS	0x40				/* turn on stat output */

extern U8              global_flag;

//
#define SERVER_CHANNEL_PORT_DEFAULT         5980

#if defined(WIN32)
#define DEFAULT_STATS_FILE			"c:/weaved/sc_config.txt"
#else
#define DEFAULT_STATS_FILE			"/tmp/sc_stats.txt"
#endif

#define DEFAULT_STATISTICS_INTERVAL 15


// Custom File config for each product here, this one is for server channel config
typedef struct server_channel_config_
{
    // Socket to listen for server channel config on
    U16			udp_listen_port;                        
	IPADDR		Bind_IP;
	SOCKET		udp_listen_soc;
    //
	int			verbose;
	int			log_level;
	int			auto_reload;
    //
    YHASH       *tags;

	char		config_file[MAX_PATH];	
    char        run_as_user[MAX_PATH];
    char        chroot[MAX_PATH];
    char        task_notify_path[MAX_PATH];

	// stats file (optional)
    unsigned int stats_interval;
	char		stats_file[MAX_PATH];	
	char		last_msg[256];

    // Stats
	long		rx_packets;                         
	long		tx_packets;
	long		tx_errors;
	long		rx_errors;
	
    long		requests;
	long		unknown_packets;
    long        rx_broadcast_packets;
	long		runt_packets;
	long		bad_requests;

    // Pidfile for daemon
	char		pidfile[MAX_PATH];
}SC_CONFIG;

#endif

