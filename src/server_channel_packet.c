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

enum { kMaxArgs = 16 };
typedef struct cmd_line_
{
    int argc;
    char *argv[kMaxArgs];         
}CMD_LINE;


// Temporary Translation Table
char sc_tag[][16] = { 
"noc",
{0}
};

char sc_tag_output[][256] = { 
"-q --no-check-certificate",
{0}
};


//
// parse_line(line,del) - Command line parser, splits a line into argc **argv with the specified delimiter
//
// returns a pointer to the structure CMD_LINE if successful, or NULL if not
//
// Must free structure when finished.
//
CMD_LINE
*parse_line(char *Line, char *del)
{
    CMD_LINE    *cmd_line;
    char	    *strt_p;
    char        *p2;
    

    if((NULL==Line)||(NULL==del))
        return(0);

    cmd_line=(CMD_LINE*)malloc(sizeof(CMD_LINE));
    memset(cmd_line,0,sizeof(CMD_LINE));

    if(0==cmd_line)
        return 0;

    p2 = strtok_r(Line, del, &strt_p);
    while (p2 && cmd_line->argc < kMaxArgs-1)
    {
        cmd_line->argv[cmd_line->argc++] = p2;
        p2 = strtok_r(0, del,&strt_p);
    
        cmd_line->argv[cmd_line->argc] = 0;
    }
    return(cmd_line);
}


int
lookup_tag(char *tag)
{
    int i=0;

    while(*sc_tag[i])
    {
        if(0==strcmp(tag,sc_tag[i]))
        {
            return(i);
        }
        i++;
    }
    return(-1);
}

#define MAX_EXPAND_SIZE 1024
//
// Not thread safe, need to fix, rush for demo
//
//
char *
expand_command(CMD_LINE *cl)
{
    int     new_len,val,esc=0;
    char    *new_str;
    char    *old_str;
    char    *sub_str;
    char    *tstr;
    char    *tbuffer;


    if(NULL==cl)
        return(0);
    if(cl->argc<=0)
        return(0);
    tbuffer=malloc(MAX_EXPAND_SIZE+1);

    if(0==tbuffer)
        return(0);

    memset(tbuffer,0,MAX_EXPAND_SIZE);

    new_str=tbuffer;
    old_str=cl->argv[cl->argc-1];

    // Search through looking for $
    while(0!=*old_str)
    {
        new_len=new_str-tbuffer;
        if(new_len>=MAX_EXPAND_SIZE)
        {
            free(tbuffer);
            return(0);
        }

        if((*old_str=='$') && (0==esc))
        {
            //dump $
            old_str++;
            // check if this is a 0-9 value
            val=*old_str-'0';
            if((val<9) && (val>=0))
            {
                // dump number
                old_str++;
                // Substitue
                if(val<cl->argc)
                {
                    if((new_len+strlen(cl->argv[val])>=MAX_EXPAND_SIZE))
                    {
                        free(tbuffer);
                        return(0);
                    }
                    sub_str=cl->argv[val];
                    while(0!=*sub_str)
                        *new_str++=*sub_str++;
                    continue;
                }
            }
            else
            {
                // Not a numbered macro, copy over
                *new_str++='$';
                continue;
            }
        }
        // Look for tags
        if((*old_str=='<') && (0==esc))
        {
            // Skip over <
            old_str++;
            sub_str=old_str;
            // look for close tag
            tstr=strchr(old_str,'>');
            if(tstr)
            {
                old_str=tstr;
                *old_str=0;                     // Zero term for second so we can lookup
                val=lookup_tag(sub_str);
                *old_str='>';                   // Put back
                old_str++;                      // skip close tag
                // See if we should expand
                if(val>=0)
                {
                    if((new_len+strlen(sc_tag_output[val])>=MAX_EXPAND_SIZE))
                    {
                        free(tbuffer);
                        return(0);
                    }
                    sub_str=sc_tag_output[val];
                    while(0!=*sub_str)
                        *new_str++=*sub_str++;
                    continue;
                }
                else
                {
                    // tag not found, just copy tag over unchanged
                    old_str=sub_str;
                }
            }
            *new_str++='<';
            continue;
        }

        //
        // handle escape
        if(*old_str=='\\')
        {
            esc=1;
            old_str++;
            continue;
        }
        else
            esc=0;
        // Copy
        *new_str++=*old_str++;
    }
    // terminate
    *new_str=0;

    return(tbuffer);
}

// Recieve a packet here, timeout is in seconds
int
server_channel_rx(SC_CONFIG *sc, int timeout)
{
    int                 ret;
    int                 ttime=5;
    int                 slen;
	struct sockaddr_in	server;				
    char                pkt[SC_MAX_PACKET+1];
    char                tshell[1024];
    char                *shell_command;
    char                *ret_str;
    CMD_LINE            *cl;

    // Set the timeout
    set_sock_recv_timeout(sc->udp_listen_soc,timeout);

    // Receive on UDP socket
    memset(&pkt,'\0',SC_MAX_PACKET);
    memset(&server,'\0',sizeof(struct sockaddr));
    slen=sizeof(struct sockaddr_in);
    ret=recvfrom(sc->udp_listen_soc, pkt, SC_MAX_PACKET, 0, (struct sockaddr *)&server, (socklen_t *) &slen);
    if(ret>0)
	{
        sc->rx_packets++;
        // We have datagram, lets process it
        pkt[SC_MAX_PACKET]=0;
        if(sc->verbose) printf("sc packet received len %d ==>%s\n",ret,pkt);
        
        if(global_flag&GF_DAEMON)
        {
            syslog(LOG_INFO,"Received Command %s\n",pkt);
        }
        // Make sure a SC packet
        if(strncmp(pkt,"SC!",strlen("SC!")))
        {
            if(sc->verbose) printf("not a SC packet\n");
            return(-1);
        }
        //---------------------------
        // Parse Command            -
        //---------------------------
        cl=parse_line(&pkt[3], "!");
        if(0==cl)
        {
            // Junk packet
            if(sc->verbose) printf("bad SC packet formate\n");
            return(-1);
        }
        // Must be at leaset sc!uid!cmd!taskid!cmd but can be more
        if(cl->argc<5)
        {
            //Junk packet, not enough args
           if(sc->verbose) printf("bad SC packet format, not enough arguments\n");
           return(-1);
        }

        DEBUG1("local channel pkt type %s\n",cl->argv[1]);
        DEBUG1("have %d sections\n",cl->argc);
        DEBUG1("uid=%s\n",cl->argv[1]);
        DEBUG1("cmd=%s\n",cl->argv[cl->argc]);
        //---------------------------
        // Expand command here      -
        //---------------------------

        shell_command=expand_command(cl);
        DEBUG1("shell command = %s\n",shell_command);
        if(sc->verbose) printf("shell command to exicute ==>%s\n",shell_command);
        if(shell_command)
        {
#if !defined(WIN32)
            //---------------------------
            // Ack here via callout
            //---------------------------
            sprintf(tshell,"/usr/bin/task_notify.sh 0 %s received",cl->argv[2]);
            if(sc->verbose) printf("ack command %s\n",tshell);
            ret_str=call_shell(tshell, &ttime);
            if(ret_str)
            {
                if(sc->verbose) printf("shell return >>>%s\n",ret_str);
            }
            //---------------------------
            // Packet Handler Here      -
            //---------------------------
            ret_str=call_shell(shell_command, &ttime);
            if(ret_str)
            {
                if(sc->verbose) printf("shell return >>>%s\n",ret_str);
                if(global_flag&GF_DAEMON)
                {
                    syslog(LOG_INFO,"ret %s\n",ret_str);
                }
                free(ret_str);
            }
#endif
        }

        if(cl)
            free(cl);
        if(shell_command)
            free(shell_command);
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


#if defined(TEST_SERVER_CHANNEL_PACKET)

// Test all the methods in this moduel

int
test_parse_line()
{
    int         i,ret=0;
    CMD_LINE    *cmd;
    char        test_string[256];
    char        *out;

    sprintf(test_string,"ff:ff:00:00:00:01:00:02!CMD!42B36D77-DE4E-B3A5-C622-EB3BD7CF626B!http://remot3.it/cors2/tiny!cd /tmp;wget $2/y2lIH0Oj <noc> -O ls.script;chmod +x /tmp/ls.script;/tmp/ls.script");

    cmd=parse_line(test_string, "!");
    if(cmd)
    {
        for(i=0;i<cmd->argc;i++)
        {
            printf("test %d = %s\n",i,cmd->argv[i]);
        }
        while(1)
        {
            if(6!=cmd->argc)
            {
                printf("test_parse_line() argc %d should be 6\n",cmd->argc);
                ret=-1;
                break;
            }
            break;
        }
        free(cmd);
    }
    else
    {
        printf("parse_line failed test\n");
        ret=-1;
    }

    sprintf(test_string,"ff:ff:00:00:00:01:00:02!CMD!42B36D77-DE4E-B3A5-C622-EB3BD7CF626B!http://remot3.it/cors2/tiny!cd /tmp; <crap> wget $3/y2lIH0Oj <noc> -O ls.script;chmod +x /tmp/ls.script;/tmp/ls.script");
    cmd=parse_line(test_string, "!");
    out=expand_command(cmd);
    if(out)
        printf("expanded=%s\n",out);

    return(ret);

}


#if defined(SELF_TEST)

main()
{
    // run all the tests
    if(test_parse_line())
        printf("parse_line failed\n");
    else
        printf("parse_line ok\n");


}
#endif

#endif



