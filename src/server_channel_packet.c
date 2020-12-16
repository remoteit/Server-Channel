/*!															   www.remot3.it			
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
 * Copyright (C) 2018 remot3.it						    -
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
#include "daemonize.h"
#include  "yhash.h"
#include "server_channel.h"
#include "server_channel_packet.h"

enum { kMaxArgs = 16 };
typedef struct cmd_line_
{
    int argc;
    char *argv[kMaxArgs];         
}CMD_LINE;

#define TASK_ID_CACHE_DEAPTH 5
#define TASK_ID_SIZE    64
char taskid_cache[TASK_ID_CACHE_DEAPTH][TASK_ID_SIZE+1];
int taskid_ptr=-1;

//
// Init taskid cache
//
void
init_taskid()
{
    memset(taskid_cache,0,sizeof(taskid_cache));
    taskid_ptr=0;
}
//
// Add taskid to cache
//
void
add_taskid(char *str)
{
    if(taskid_ptr<0)
        init_taskid();
    strncpy(taskid_cache[taskid_ptr++],str,TASK_ID_SIZE);

    if(taskid_ptr>=TASK_ID_CACHE_DEAPTH)
        taskid_ptr=0;
    return;
}

//
// Check if taskid is in cache. 1 if in cache 0 if not
//
int
check_taskid_cache(char *str)
{
    int i,ret=0;
    if(taskid_ptr<0)
        init_taskid();
    else
    {
        if(str)
        {
            for(i=0;i<TASK_ID_CACHE_DEAPTH;i++)
            {
                DEBUG1("checking cache(%d) %s vs input %s\n",i,taskid_cache[i],str);
                if(0==strcmp(taskid_cache[i],str))
                {
                    ret=1;
                    break;
                }
            }
        }
    }
    return(ret);
}

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




#define MAX_EXPAND_SIZE 1024
//
// Not thread safe, need to fix, rush for demo
//
//
char *
expand_command(SC_CONFIG *sc,CMD_LINE *cl)
{
    int     new_len,val,esc=0;
    char    *new_str;
    char    *old_str;
    char    *sub_str;
    char    *tstr;
    char    *tbuffer;
    char    *xlate_val;


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
                xlate_val=(char*)yhash_lookup_string(sc->tags, sub_str);
                //val=lookup_tag(sub_str);
                *old_str='>';                   // Put back
                old_str++;                      // skip close tag
                // See if we should expand
                //if(val>=0)
                if(xlate_val)
                {
                    if((new_len+strlen(xlate_val)>=MAX_EXPAND_SIZE))
                    {
                        free(tbuffer);
                        return(0);
                    }
                    sub_str=xlate_val;
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
    char                tshell[8092];
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
        //-----------------------------------------
        // Parse Command  (do not iclude SC!      -
        //-----------------------------------------
        cl=parse_line(&pkt[3], "!");
        if(0==cl)
        {
            // Junk packet
            if(sc->verbose) printf("bad SC packet formate\n");
            return(-1);
        }
        // Must be at leaset uid!cmd!taskid!api!cmd but can be more (note we don not parse sc!)
        if(cl->argc<5)
        {
            //Junk packet, not enough args
           if(sc->verbose) printf("bad SC packet format, not enough arguments (wanted at least 5, got %d)\n",cl->argc);
           return(-1);
        }

        DEBUG1("local channel pkt type %s\n",cl->argv[0]);
        DEBUG1("have %d sections\n",cl->argc);
        DEBUG1("uid=%s\n",cl->argv[1]);
        DEBUG1("cmd=%s\n",cl->argv[cl->argc]);
        //---------------------------
        // Expand command here      -
        //---------------------------

        shell_command=expand_command(sc,cl);
        DEBUG2("shell command = %s\n",shell_command);
        if(sc->verbose) printf("shell command to exicute ==>%s\n",shell_command);
        if(shell_command)
        {
#if !defined(WIN32)
            //---------------------------
            // Ack here via callout
            //---------------------------
            sprintf(tshell,"%s 0 %s %s received",sc->task_notify_path,cl->argv[2],cl->argv[3]);
            if(sc->verbose) printf("ack command %s\n",tshell);
            ret_str=call_shell(tshell, &ttime);
            if(ret_str)
            {
                if(sc->verbose) printf("shell (task_notify) return >>>%s\n",ret_str);
            }
            // 
            // Check if already exicuted this, we skip calling again if we have called this taskid before
            //

            if(0==check_taskid_cache(cl->argv[2]))
            {
                //---------------------------
                // Packet Handler Here      -
                //---------------------------
                ret_str=call_shell(shell_command, &ttime);
                if(ret_str)
                {
                    if(sc->verbose) printf("shell (command) return >>>%s\n",ret_str);
                    if(global_flag&GF_DAEMON)
                    {
                        syslog(LOG_INFO,"ret %s\n",ret_str);
                    }
                    free(ret_str);
                }
                // Add taskid to cache, we do not want to shell out again
                add_taskid(cl->argv[2]);
            }
            else
            {
                if(sc->verbose) printf("Task id %s already started, just ack.\n",cl->argv[2]);
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
    SC_CONFIG   sc;

    memset(&sc,'\0',sizeof(SC_CONFIG));
    sc.tags=yhash_init(YHASH_MAP,8);
    yhash_insert_string_key(sc.tags,"noc","expanded noc tag");
    yhash_insert_string_key(sc.tags,"test","expanded test tag");


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
            if(5!=cmd->argc)
            {
                printf("test_parse_line() argc %d should be 5\n",cmd->argc);
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

    sprintf(test_string,"ff:ff:00:00:00:01:00:02!CMD!42B36D77-DE4E-B3A5-C622-EB3BD7CF626B!remot3.it/cors2!http://remot3.it/cors2/tiny!cd /tmp; <test> wget $3/y2lIH0Oj <noc> -O ls.script;chmod +x /tmp/ls.script;/tmp/ls.script");
    cmd=parse_line(test_string, "!");
    out=expand_command(&sc,cmd);
    if(out)
        printf("expanded=%s\n",out);

    yhash_destroy(sc.tags, 0);

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



