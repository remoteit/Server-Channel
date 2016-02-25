/* 																www.yoics.com
 *---------------------------------------------------------------------------
 *! \file file_config.c
 *  \brief Configuration file reader 
 *																			
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 3, 2006									-        
 *
 *---------------------------------------------------------------------------    
 *                                                             				-
 * Copyright (C) 2006, Yoics Inc, www.yoics.com								-
 *                                                                         	-
 * $Date: 2006/08/29 20:35:55 $
 *
 *---------------------------------------------------------------------------
 *
 * Notes:
 *
 * 
 *
*/

#include <stdarg.h>
#include "config.h"
#include "file_config.h"
#include "log.h"
#include "debug.h"
#include "yhash.h"


// config file strings
#define		SC_TASK_NOTIFY  	 		"Task_Notify_Script"
#define		SC_STATS_FILE  	 			"Stats_File"
#define     SC_STATS_INTERVAL           "Stats_Interval"
#define		SC_LISTEN_PORT				"Listen_Port"
#define     SC_BIND_IP                  "Bind_IP"
#define     SC_RUN_AS_USER              "Run_As_User"
#define     SC_TAG                      "tag"

int
readln_from_a_file(FILE *fp, char *line, int size)
{
	char *p;

    do
      p = fgets( line, size, fp );
    while( ( p != NULL ) && ( *line == '#') );

    if( p == NULL )
		return( 0 );

    if (strchr(line, '\n'))
          *strchr(line, '\n') = '\0';
    if (strchr(line, '\r'))
          *strchr(line, '\r') = '\0';
    return( 1 );
}

#define MAX_LINE_SIZE	2048
//
//
//
int
read_file_config(char *file, SC_CONFIG *config)
{
	U8		line[MAX_LINE_SIZE];
	char	*subst;
	char	*strt_p;
	int		ret=0;
	FILE	*fp;

    if( (0==file) || (0==strlen(file)) )
    {
        return(-1);
    }
	// Read from file
	if(NULL == (fp = fopen( (char *) file, "r")) )
	{
		ret=0;
	}
	else
	{	
		while(readln_from_a_file(fp, (char *) line, MAX_LINE_SIZE-4))
		{
			if(strlen((char *) line)==0)
				continue;

			subst= strtok_r((char *) line," \n",&strt_p);

			// Get Rid of whitespace
			while(*subst==' ')
				subst++;

			DEBUG1("readcmd->%s\n",subst);


			if(strlen( (char *) subst)==0)
			{
				// do nothing
			}
			else if(0==strcmp( (char *) subst,SC_TASK_NOTIFY))
			{
				//
				subst=strtok_r(NULL," \n",&strt_p);
				strncpy((char *) config->task_notify_path, (char *) subst, MAX_PATH);
				config->task_notify_path[MAX_PATH-1]=0;
			}
            else if(0==strcmp( (char *) subst,SC_RUN_AS_USER))
            {
                //
                subst=strtok_r(NULL," \n",&strt_p);
                strncpy((char *) config->run_as_user, (char *) subst, MAX_PATH);
                config->run_as_user[MAX_PATH-1]=0;
            }
            else if(0==strcmp( (char *) subst,SC_STATS_FILE))
            {
                //
                subst=strtok_r(NULL," \n",&strt_p);
                strncpy((char *) config->stats_file, (char *) subst, MAX_PATH);
                config->stats_file[MAX_PATH-1]=0;
            }
			else if(0==strcmp((char *) subst,SC_LISTEN_PORT))
			{
				//
				subst=strtok_r(NULL," \n",&strt_p);
				config->udp_listen_port=atoi((char*)subst);
				DEBUG1("udp port set to %d\n",config->udp_listen_port);
			}
			else if(0==strcmp((char *) subst,SC_BIND_IP))
			{
			    //
                subst=strtok_r(NULL,".\n",&strt_p);
                if(strlen((char *) subst))
                    config->Bind_IP.ipb1=atoi((char *) subst);
                subst=strtok_r(NULL,".\n",&strt_p);
                if(strlen((char *) subst))
                    config->Bind_IP.ipb2=atoi((char *) subst);
                subst=strtok_r(NULL,".\n",&strt_p);
                if(subst)
                    config->Bind_IP.ipb3=atoi((char *) subst);
                subst=strtok_r(NULL,".\n",&strt_p);
                if(subst)
                    config->Bind_IP.ipb4=atoi((char *) subst);
                DEBUG1("Bind IP set to %d.%d.%d.%d\n",config->Bind_IP.ipb1, config->Bind_IP.ipb2,
                    config->Bind_IP.ipb3, config->Bind_IP.ipb4);
			}
			else if(0==strcmp((char *) subst,SC_TAG))
			{
                char tag[16],*tstr;
				// Get the tag first
				subst=strtok_r(NULL," \n",&strt_p);
                if(subst)
                {
                    strncpy(tag,(char*)subst,15);
                    tag[15]=0;
				    // Get the value, all the way to the end
                    subst=strtok_r(NULL,"\n",&strt_p);
                    if(subst)
                    {
                        /*
                       value=(char*)malloc(strlen((char*)subst)+1);
                       if(0==value)
                       {
                           if(config->verbose)
                               printf("malloc error on tag add function %s, File %s line %d\n",__func__, __FILE__,__LINE__);
                           DEBUG1("malloc error on tag add function %s, File %s line %d\n",__func__, __FILE__,__LINE__);
                           continue;
                       }
                       strcpy(value,(char*)subst);
                        value[255]=0;
                        */
                        // Got value, lookup tag
                        tstr=(char*)yhash_lookup_string(config->tags, tag);
                        if(tstr)
                        {
                            // Already there delete
                            tstr=(char*)yhash_delete_string(config->tags, tag);
                            free(tstr);
                        }
                        if(yhash_insert_string_key(config->tags, tag,subst))
                        {
                            // Failed insert free value
                            DEBUG2("Failed to insert tag %s value %s\n",tag,subst);
                        }
                    }
                }

			}
		}
		ret=1;
		fclose(fp);
	}
	return(ret);
}



