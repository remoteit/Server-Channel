/*!																www.yoics.com			
 *---------------------------------------------------------------------------
 *! \file arch.c
 *  \brief Platform dependent code for yoics system, contains abstaracted 
 *		patform independent interface interface.
 *																			
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 3, 2006									-        
 *
 *---------------------------------------------------------------------------    
 * Version                                                                  -
 * 0.1 Original Version August 31, 2006     							    -
 *																			-
 * (c)2006 Yoics Inc. All Rights Reserved									-
 *---------------------------------------------------------------------------
 *
 */
#if !defined(WIN32)
#include <sys/types.h>
#include <dirent.h>
#endif
#include	"config.h"
#include	"mytypes.h"
#include	"arch.h"
#include	"debug.h"

static int		yrandom_init=0;

#if defined(WIN32) || defined(WINCE)
#define		timeb	_timeb
#define		ftime	_ftime
#endif








//
// Convert a UID in asc format to binary (ie this format 00:00:2D:4A:7B:3E:1F:8B)
//
// Should this be non distructive?
//
void
UID_Extract(U8 *uid,U8 *uid_ascii)
{
	int	i;
	char	*subst,*strt_p;
	U8		*in=uid_ascii;

	// remove whitespace
	while(('\n'==*in) || ('\r'==*in) || (' '==*in))
		in++;

	// scan out the bytes.
	subst=strtok_r( (char *)in,": \0\n",&strt_p);

	// Loop for 8 bytes
	if(subst)
	{
		for(i=0;i<8;i++)
		{
			uid[i]=	(U8)strtol((char *) subst,(char **)NULL, 16);
			subst= strtok_r(NULL,": \n\0",&strt_p);
			if(NULL==subst)
				break;
		}
	}
	return;
}


void
strtolower(char *string)
{
	int i;

	if(string)
	{
		for (i = 0; string[i]; i++)
			string[i] = tolower(string[i]);
	}
}

//
// Returns a 32bit seconds count
//
U32 second_count(void)
{
	int	seconds;
	// grab a time value and return a 16 bit 10ms count used for timestamping - 1000=1second
	//#if 0
	//ftime() is obsolete everywhere except maybe embedded Linux?  
	//OS X didn't get ftime() until OS X 10.4.  So we do this...
	// BUT it doesn't work with Mike's code.  It does now....  defined(LINUX) ||
#if  defined(MACOSX) || defined(__ECOS)
	struct timeval   Tp;
	struct timezone  Tzp;
    gettimeofday( &Tp, &Tzp );     /* get timeofday */
	seconds=0;
	seconds=(U32)(Tp.tv_sec);
#endif
#if defined(LINUX) || defined(WIN32)
	struct timeb    timebuffer;
	ftime( &timebuffer );
	seconds=0;
	seconds=(U32)(timebuffer.time);
#endif
#if defined(WINCE)
	SYSTEMTIME tSystemtime;
	GetLocalTime(&tSystemtime);
	seconds=(U32)tSystemtime.wSecond;
#endif
	return(seconds);
}

//
// hund_ms_count(void) - returns a 16 bit 100ms tick count.
//
U16	hund_ms_count(void)
{	
	U16	ticks;
	// grab a time value and return a 16 bit 10ms count used for timestamping - 1000=1second
	//#if 0
	//ftime() is obsolete everywhere except maybe embedded Linux?  
	//OS X didn't get ftime() until OS X 10.4.  So we do this...
	// BUT it doesn't work with Mike's code.  It does now....  defined(LINUX) ||

// Should this all be like the MAC?

#if  defined(MACOSX) || defined(__ECOS) || defined(LINUX)
	struct timeval   Tp;
	struct timezone  Tzp;
    gettimeofday( &Tp, &Tzp );     /* get timeofday */
	ticks=0;
	ticks=(U16)(Tp.tv_sec*10);
	ticks=ticks + ((Tp.tv_usec)/100000);
#endif
#if defined(WIN32) 
	struct timeb    timebuffer; 
	ftime( &timebuffer );
	ticks=0;
	ticks=(U16)(timebuffer.time*10);
	ticks=ticks + ((timebuffer.millitm)/100);
#endif
#if defined(WINCE)
	SYSTEMTIME tSystemtime;
	GetLocalTime(&tSystemtime);
	ticks=0;
	ticks=(U16)(tSystemtime.wSecond*10);
	ticks=ticks+(tSystemtime.wMilliseconds/100);
#endif
	return(ticks);
}



//
// threadswitch() - force a threadswitch
//
void
threadswitch()
{
#if defined(WIN32) || defined(WINCE)
			Sleep(0);
#endif

#if defined(MACOSX)
			usleep(100);
			sched_yield();
#endif

#if defined(LINUX)
			sched_yield();

#endif
#if defined(__ECOS)
			cyg_thread_delay(1);		
#endif
}


//
// Second Sleep Platform Independent
//
void ysleep_seconds(U16 duration)
{
#if defined(WIN32) || defined(WINCE)
			Sleep(duration*1000);  
#endif

#if defined(LINUX) || defined (MACOSX)
			sleep(duration);
#endif
#if defined(__ECOS)
			cyg_thread_delay(duration*1000);		
#endif
}

//
// Second Sleep Platform Independent
//
void ysleep_usec(U32 duration)
{
#if defined(WIN32) || defined(WINCE)
			Sleep(duration/100);  
#endif

#if defined(LINUX) || defined (MACOSX)
			usleep(duration);
#endif
}

//
//
//
size_t file_length(char *filename)
{
	struct	stat st;
	size_t	size=0;
	int		ret;
    //int     i=0;
	
	if(0==filename)
	{
		DEBUG9("file bad name\n");
		return(0);
	}

	ret=stat(filename, &st);

	if(ret>=0)
	{
		size = st.st_size;
		DEBUG9("file size is %d for %s stat ret %d\n",st.st_size,filename,ret);
	}
	else
	{

#if defined(DEBUG0)
		DEBUG0("stat fail for filename %s ",filename);
		//perror("stat fail \n");
#endif
		size=0;
	}
	return(size);
}

#if 0
long file_length(char *filename)
{
#if defined (WIN32)
	struct	_stat st;
#else
	struct	stat st;
#endif
	long	size;
	int		ret;

	ret=stat(filename, &st);			// Fix for windows

	if(ret>=0)
	{
		size = st.st_size;
	}
	else
	{

#if defined(DEBUG0)
		DEBUG0("stat fail for filename %s ",filename);
		//perror("stat fail \n");
#endif
		size=0;
	}
	return(size);

}
#endif

//
// Strip Extra slashes, IE /// = / or // = /, but / = /
//
int
strip_slash(char *buffer)
{
	char *d=buffer;
	char *s=buffer;

	while(*s)
	{
		if('/'==*s)
		{
			*d++=*s++;
			while('/'==*s)
				s++;
			continue;
		}
		*d++=*s++;
	}
	*d=*s;

	return(0);
}


void
strip_crlf(char *d)
{
	if(d==0)
		return;

	// chop off any \r\n at the end of a string
	while(strlen(d) && (('\n'==d[strlen(d)-1]) || ('\r'==d[strlen(d)-1]) ) )
		d[strlen(d)-1]=0;
}

/*
//
// Doesn't work in windows
//
U32 file_length(char *filename)
{
FILE *fp;
U32 pos;
U32 end;
	
	if(NULL == (fp = fopen( (char *) filename, "r")) )
	{
		return(0);
	}

	pos = ftell (fp);
	fseek (fp, 0, SEEK_END);
	end = ftell (fp);
	fseek (fp, pos, SEEK_SET);
	fclose(fp);

	return end;
}
*/

//
// Random number Generator
//
#if defined(BADRAND)
static unsigned long next = 1;

/* RAND_MAX assumed to be 32767 */
#define MY_RAND_MAX	32767
int myrand(void) 
{
  int t;
  next = next * 1103515245L + 12345L;
  t=(unsigned int)((next >> 16) & 0x7fff);
  DEBUG1("myrand = %d  next %ld\n",t,next);
  return (t); 

//   next = next * 1103515245 + 12345;
//   return((unsigned)(next/65536) % 32768);
}

void mysrand(unsigned seed) 
{
	DEBUG1("seed = %d\n",seed);
   next = seed;
}

#endif


#if defined(BADRAND)

void yrand_seed(long seed)
{
	mysrand(second_count()^seed);
	yrand(10);
}

int yrand(int max)
{
	if(yrandom_init!=1)
		yrand_seed(hund_ms_count()+second_count());
//return(1 + (int) (max * (myrand() / (RAND_MAX + 1.0))));
	return(myrand()%max);
}


#else

void yrand_seed(long seed)
{
	yrandom_init=1;
	srand((hund_ms_count()+second_count())^seed);
	yrand(10);
}

int yrand(int max)
{
	if(yrandom_init!=1)
		yrand_seed(hund_ms_count());
	
	return(1 + (int) (max * (rand() / (RAND_MAX + 1.0))));
}

#endif


//
// isDirectoryNotEmpty(char *dirname)
//
// Returns 1 if directory not empty.
// Returns 0 if directory empty
// Returns -1 if not a directory or doesn't exist
//
int 
isDirectoryNotEmpty(char *dirname) 
{
#if defined(LINUX) || defined(MACOSX) || defined(IOS) || defined(BSD_TYPE)
  int n = 0;
  struct dirent *d;

  DIR *dir = opendir(dirname);

  if (dir == NULL) //Not a directory or doesn't exist
    return -1;

  while ((d = readdir(dir)) != NULL) 
  {
	// Likely we should verify that there are files, not just directories
    if(++n > 2)
      break;
  }
  closedir(dir);
  
  if (n > 2) //Directory Empty
  {
	  return 1;
  }
  else
  {
	  return 0;
  }
#endif
#if defined(WIN32)
  return -1;
#endif
}


//
// DeleteDirectroyFiles(char *dirname)
//
// Deletes all files in a directory (in linux only files, no symlinks etc..)
//
// Returns >0 number of files deleted.
// Returns 0 if directory empty
// Returns -1 if not a directory or doesn't exist
//
int DeleteDirectroyFiles(char *dirname)
{
  int count = 0;

  #if defined(LINUX)
  struct dirent *d;
  char		tmp_path[PATH_MAX];


  DIR *dir = opendir(dirname);

  struct stat buf;

  if (dir == NULL) //Not a directory or doesn't exist
  {
		DEBUG0("DIR NULL\n");
		return -1;
  }
  //
  // Loop Through files and unlink them
  //
  while ((d = readdir(dir)) != NULL) 
  {
	  // If not directory delete
	  if(d->d_type==DT_REG)
	  {
		  sprintf(tmp_path,"%s/%s",dirname,d->d_name);
		  DEBUG0("unlink file %s - DT_REG method\n",tmp_path);
		  count++;
		  unlink(tmp_path);
	  } 
	  else if (d->d_type==DT_UNKNOWN )
	  {
		  // Use stat method
		  if(0==lstat(d->d_name, &buf))
		  {
			  if((buf.st_mode & S_IFMT) == S_IFREG )
			  {
				  sprintf(tmp_path,"%s/%s",dirname,d->d_name);
				  DEBUG0("unlink file %s - LSTAT method\n",tmp_path);
				  count++;
				  unlink(tmp_path);
			  }
		  }
	  }

  }
  closedir(dir);
#endif

#if defined(WIN32)
  count=-1; // not implemented yet
  /*
  // Use stat method
	if(0==_stat(dirp->d_name, &buf))
	{
		if(buf.st_mode==S_ISREG)
		{
			DEBUG0("unlink file %d - _stat method\n",d->d_name);
			count++;
			unlink(d->d_name);
		}
	}
	*/
#endif

  // Return the number of files deleted
  return(count);
}


// Trim the beginning and end of string
void trim(char * s) 
{
    char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(s, p, l + 1);

}

//
// Replace all characters of value orig_char with replace_char in a string, returns the number of chars replaced.
//
int
str_char_replace(char *s, const char orig_char, const char replace_char)
{
	char * p = s;
	int		count=0;

	while(*p)
	{
		if(orig_char==*p)
		{
			*p=replace_char;
			count++;
		}
		p++;
	}

	return(count);
}


//
// updated_str=repl_str(repl_str(const char *str, const char *search_sting, const char *replace_str) 
//
// Replace all the occerences of old with new in a string, returns the new string
// that must be free'd when not used anymore.
//
char *repl_str(const char *str, const char *old_str, const char *new_str) 
{

	/* Adjust each of the below values to suit your needs. */

	/* Increment positions cache size initially by this number. */
	size_t cache_sz_inc = 16;
	/* Thereafter, each time capacity needs to be increased,
	 * multiply the increment by this factor. */
	const size_t cache_sz_inc_factor = 2;
	/* But never increment capacity by more than this number. */
	const size_t cache_sz_inc_max = 1048576;

	char *pret, *ret = NULL;
	const char *pstr2, *pstr = str;
	size_t i, count = 0;
	ptrdiff_t *pos_cache = NULL;
	size_t cache_sz = 0;
	size_t cpylen, orglen, retlen, newlen, oldlen;

    // No Null Strings Allowed
    if((0==str) || (0==old_str) || (0==new_str))
        return(0);

    if( (0==strlen(str)) || (0==strlen(old_str)))
        return(0);
    oldlen = strlen(old_str);

	/* Find all matches and cache their positions. */
	while ((pstr2 = strstr(pstr, old_str)) != NULL) 
    {
		count++;

		/* Increase the cache size when necessary. */
		if (cache_sz < count) {
			cache_sz += cache_sz_inc;
			pos_cache = realloc(pos_cache, sizeof(*pos_cache) * cache_sz);
			if (pos_cache == NULL) {
				goto end_repl_str;
			}
			cache_sz_inc *= cache_sz_inc_factor;
			if (cache_sz_inc > cache_sz_inc_max) {
				cache_sz_inc = cache_sz_inc_max;
			}
		}

		pos_cache[count-1] = pstr2 - str;
		pstr = pstr2 + oldlen;
	}

	orglen = pstr - str + strlen(pstr);

	/* Allocate memory for the post-replacement string. */
	if (count > 0) {
		newlen = strlen(new_str);
		retlen = orglen + (newlen - oldlen) * count;
	} else	retlen = orglen;
	ret = malloc(retlen + 1);
	if (ret == NULL) {
		goto end_repl_str;
	}

	if (count == 0) {
		/* If no matches, then just duplicate the string. */
		strcpy(ret, str);
	} else {
		/* Otherwise, duplicate the string whilst performing
		 * the replacements using the position cache. */
		pret = ret;
		memcpy(pret, str, pos_cache[0]);
		pret += pos_cache[0];
		for (i = 0; i < count; i++) {
			memcpy(pret, new_str, newlen);
			pret += newlen;
			pstr = str + pos_cache[i] + oldlen;
			cpylen = (i == count-1 ? orglen : pos_cache[i+1]) - pos_cache[i] - oldlen;
			memcpy(pret, pstr, cpylen);
			pret += cpylen;
		}
		ret[retlen] = '\0';
	}

end_repl_str:
	/* Free the cache and return the post-replacement string,
	 * which will be NULL in the event of an error. */
	free(pos_cache);
	return ret;
}




#if (defined(WIN32) || !defined(_GNU_SOURCE))
char *strcasestr(const char *haystack, const char *needle)
{
	int		i,j;
	char	*tptr=NULL;
	char	c1,c2;
	int		nlen = strlen(needle);
	int		hlen = strlen(haystack);

	j=0;
	for(i=0;i<hlen;i++)
	{
		c1=haystack[i];
		c2=needle[j];
		if (toupper(c1) == toupper(c2))
		{
			j++;
			if(j==nlen)
			{	
				tptr=(char *)&haystack[i-nlen];
				break;
			}
		}
		else
			j=0;
	}

	return(tptr);
}
#endif


#if defined(LINUX) || defined(MACOSX)
//
// Returns NULL for failed, or string (must free) on result, will bail on first read line
// 
// Returns:
//      Null for fail, or ptr to string with return info
//      timeout = -1 on timeout or how many seconds it took to upload
//
char *
call_shell(char *command, int *timeout)
{
    FILE    *proc;
    int     fd,flags,count=0;
    char    *line=0;

    TRACEIN;

    proc= popen(command, "r");

    if (proc != NULL) 
    {
        fd = fileno(proc);
        line=malloc(2048);

        flags = fcntl(fd, F_GETFL, 0);
        flags |= O_NONBLOCK;
        fcntl(fd, F_SETFL, flags);


        while (1) 
        {
            //int len=0;
            DEBUG3(".\n");
            if(fgets(line, 2048, proc))
            {
                DEBUG3("got len %d for line %s\n",strlen(line),line);
                // Got Line    
                *timeout=count;
                break;
            }
            else
            {
                DEBUG3("!\n");
                ysleep_seconds(1);                  // Must be 1 second because of timeout count
                count++;
                if(count>*timeout)
                {
                    DEBUG3("Timeout\n");
                    free(line);
                    line=0;
                    *timeout=-1;
                    TRACE("Timeout Shell %s",command);
                    break;
                }
            }
        }
    }
    else
    {
        *timeout=-2;
    }
    pclose(proc);

    TRACEOUT;
    return(line);
}
#endif


// Returns buffer to next line, line copied in line returns pointer to next line or NULL if no more in buffer
char *
readln_from_a_buffer(char* buffer, char *line, int size)
{
	char *p=buffer;
    char *l=line;
    int i;
    
    *line=0;
    if( NULL == p )
        return (0);

    // Skip Comments
    while( ( *p != 0 ) && ( *p == '#') )
    {
        // flush this line
        while( ( '\r' != *p )  && ( '\n' != *p ) && ( 0 != *p ) )
            p++;
        while(('\r'==*p)|| ( '\n'==*p ))
            p++;
    }
    //
    if( *p == 0 )
		return( NULL );
    // Copy Line
    i=0;
    while( ( '\r' != *p )&&( '\n' != *p )&&( 0 != *p ) )
    {
        // exit if we've reached size (IE size of 1 we would never store, because we have to do terminator)
        i++;
        if(i>=size)
            break;
        *l++=*p++;
    }
    // terminate line
    *l=0;
    // flush rest of line this line if we still have non end/ret
    if(( '\r' != *p )&&( '\n' != *p )&&( 0 != *p ) )
    {
        while( ( '\r' != *p )||( '\n' != *p )||( 0 != *p ) )
            p++;
    }
    // flush returns
    while(('\r'==*p) || ( '\n'==*p ))
        p++;

    // Return new pointer to next line
    if(*p)
        return(p);
    else
        return(NULL);
}
