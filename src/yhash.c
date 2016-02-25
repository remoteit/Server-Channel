/*!																www.yoics.com			
 *---------------------------------------------------------------------------
 *! \file yhash.c
 *  \brief Simple Fast Table hash lookup
 *																			
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 3, 2009									-        
 *
 *---------------------------------------------------------------------------    
 *																			-
 * (c)2009 Yoics Inc. All Rights Reserved									-
 *---------------------------------------------------------------------------
 *
 */

#include	"config.h"
#include	"mytypes.h"
#include    "log.h"
#include	"debug.h"
#include	"yhash.h"


//
// We should be able to specify size here and not just use the default, we should also hae a rebuild to increase or decrease the size
//

YHASH *yhash_init(int type, int size)
{
YHASH	*new_hash;
int		hash_size;
int     hash_size_raw=YHASH_DEFAULT_LOOKUP_TABLE_SIZE;

    if(size>0)
    {
        // Limit to 4 million at this point
        if(size>22)
            return(0);
        hash_size_raw=size;
    }

    hash_size=1<<hash_size_raw;
	
	new_hash=(YHASH*)malloc(1+sizeof(YHASH)+(hash_size*sizeof(YHASH_OBJ *)));

	if(new_hash)
	{
		// clean it first
		memset((void*)new_hash,'\0',sizeof(YHASH)+(hash_size*sizeof(YHASH_OBJ *)));
		new_hash->elements=0;
		new_hash->mask=(1<<hash_size_raw)-1;		// This creates a mask
		new_hash->hash_size=hash_size_raw;			// This is X in the 2^X size
        new_hash->type=type;                        // Set the hash type.
	}

	return(new_hash);
}

//
// If callback is specified, this callback will be called for every object to so they can be cleaned up.
//
int
yhash_destroy(YHASH *hash, void (*callback)(void *))
{
long		size,i;
YHASH_OBJ	*object,*tobject;

	if(hash)
	{
		size=1<<hash->hash_size;
		// We need to walk the list and free all the elements
		for(i=0;i<size;i++)
		{
			if(hash->hash[i])
			{
				// Walk the chain
				object=hash->hash[i];

                while(object)
				{
					tobject=object;
					object=object->next;
					//
					// Free tobject, by callback if specified, or if MAP type, and callback not specified, free it here.
                    //
                    if(callback)
						callback(tobject->data);
                    else if(YHASH_MAP==hash->type)
                        free(tobject->data);

					free(tobject->key);
					free(tobject);
				}
			}
		}

		// Then free the hash itself
		free(hash);
		return(0);
	}
	else
		return(-1);
}

#define CRC_GOOD_VALUE		0xf0b8
//																www.mycal.com
//---------------------------------------------------------------------------
// Simple and fast CRC16 routine for embedded processors.
//	Just slightly slower than the table lookup method but consumes
//	almost no space.  Much faster and smaller than the loop and
//	shift method that is widely used in the embedded space. 
//	Can be optimized even more in .ASM
//
//	data = (crcvalue ^ inputchar) & 0xff;
//	data = (data ^ (data << 4)) & 0xff;
//	crc = (crc >> 8) ^ ((data << 8) ^ (data <<3) ^ (data >> 4))
//---------------------------------------------------------------------------
U16
crcadd(U16 crcvalue, U8 c)
{
U16	b;

   b = (crcvalue ^ c) & 0xFF;
   b = (b ^ (b << 4)) & 0xFF;				
   b = (b << 8) ^ (b << 3) ^ (b >> 4);
   
   return((crcvalue >> 8) ^ b);
}

//
// Calculate a 4 byte hash based on o key
//
U32 calc_hash(char *key, int len)
{
int i;
U32 hash=0;

	for(i=0;i<len;i++)
	{
		// Not perfect but seems to do pretty good over a range of keys
		hash=(hash)+((hash<<15)^crcadd((hash&0xffff),key[i]));
	}
	return(hash);
}

//
// Inserts -2= key collision
//
int
yhash_insert_buffer_key(YHASH *hash, char *key, int keylen, void *value)
{
	int ret=-1;
	YHASH_OBJ *object;
	long hash_value=0;

	if(NULL==hash)
		return(ret);

	// Does it exist?
	object=yhash_lookup_object(hash, key, keylen);
	hash->lookups--;		// Fix the stats, we don't count this lookup
	if(object)
		return(-2);

	hash->lookup_fails--;  // Fix the stats for this above fail

	// Calculate raw hash
	hash_value=calc_hash(key,keylen);

	// Malloc element
	object=(YHASH_OBJ*)malloc(1+sizeof(YHASH_OBJ));

	if(object)
	{
		// Fill in object
		object->key=(char*)malloc(1+(keylen*sizeof(char)));
		if(object->key)
		{
			// copy key
			memcpy(object->key,key,keylen);
			object->key_len=keylen;
			// Store index (is this needed?)
			object->hash_index=(hash_value&hash->mask);
			// Copy Value Pointer
			object->data=value;
			object->next=0;

			// We have a good object, lets insert it
			if(0==hash->hash[object->hash_index])
			{
				DEBUG2("Insert at location %x\n",object->hash_index);
				// Hash table at this location is empty, Just insert
				hash->hash[object->hash_index]=object;
			}
			else
			{
				DEBUG2("Insert at location %x, chained\n",object->hash_index);
				// Overflow, lets chain this on the front of the list
				object->next=hash->hash[object->hash_index];
				hash->hash[object->hash_index]=object;
				hash->insert_chains++;							// insert chains counter
			}
			//
			// Last lest update the hash counters
			//
			hash->elements++;
			hash->inserts++;
			ret=0;
		}
		else
		{
			// Fail
			free(object);
		}
	}
	return(ret);
}

int
yhash_insert_string_key(YHASH *hash, char *key, void *value)
{
    char    *tvalue=(char*)value;

    if(YHASH_MAP==hash->type)
    {
        // copy value
        tvalue=(char*)malloc(1+strlen((char*)value));
        if(tvalue)
        {
            strcpy(tvalue,(char*)value);
        }
        else
            return(-1);

    }
	return(yhash_insert_buffer_key(hash, key,strlen(key), tvalue));
}



YHASH_OBJ *yhash_lookup_object(YHASH *hash, char *key, int keylen)
{
	YHASH_OBJ *object;
	long hash_value=0;

	if(NULL==hash)
		return(0);

	hash->lookups++;

	// Calculate raw hash
	hash_value=calc_hash(key,keylen);

	// Lookup 
	object=hash->hash[hash_value & hash->mask];

	while(object)
	{
		if(keylen==object->key_len)
		{
			if(0==memcmp(key,object->key,keylen))
			{
				// Found
				return(object);
			}
		}
		object=object->next;		
	}
	hash->lookup_fails++;
	return(0);
}


void *
yhash_lookup_buffer(YHASH *hash, char *key, int keylen)
{
YHASH_OBJ * object;

	object=yhash_lookup_object(hash, key, keylen);

	if(object)
		return(object->data);
	else
		return(0);
}

void *
yhash_lookup_string(YHASH *hash, char *key)
{
	return(yhash_lookup_buffer(hash, key, strlen(key)));
}



//
// Delete key / value, warning noting is done with the actual value, a pointer to it will returned if deleted, or NULL if not
//
void *
yhash_delete_buffer(YHASH *hash, char *key, int keylen)
{
YHASH_OBJ * object,*tobject;
void		*value=NULL;

	if(NULL==hash)
		return(0);

	object=yhash_lookup_object(hash, key, keylen);

	// If object it exists!  Lets Delete it
	if(object)
	{
		hash->deletes++;
		hash->elements--;
		if(object==hash->hash[object->hash_index])
		{
			// At head of list, remove
			hash->hash[object->hash_index]=object->next;
			//
			// Copy over value pointer
			value=object->data;
			//
			// Free memory
			free(object->key);
			free(object);
		}
		else
		{
			// On chain, find and remove
			tobject=hash->hash[object->hash_index];
			while(tobject)
			{
				if(tobject->next==object)
				{
					// Found, remove first
					tobject->next=object->next;
					//
					// Copy over value pointer
					value=object->data;
					//
					// Free memory
					free(object->key);
					free(object);
					break;
				}
				tobject=tobject->next;
			}
		}
	}
	return(value);
}

void *
yhash_delete_string(YHASH *hash, char *key)
{
	return(yhash_delete_buffer(hash, key, strlen(key)));
}

void
yhash_print_stats(YHASH *hash)
{
	printf("Inserts %ld \tChained %ld\n",hash->inserts,hash->insert_chains);
	printf("Lookups %ld \tFails %ld\n",hash->lookups,hash->lookup_fails);
	printf("deletes %ld \telements %ld\n",hash->deletes,hash->elements);
}





#if defined(TEST_YHASH)
	fe.filter_hash=yhash_init();
	hash_insert_buffer_key(fe.filter_hash,"abcdefg",strlen("abcdefg"),"hello");
	hash_insert_buffer_key(fe.filter_hash,"xyzzy",strlen("xyzzy"),"hello1");
	hash_insert_buffer_key(fe.filter_hash,"xyzzy1",strlen("xyzzy1"),"hello2");
	hash_insert_buffer_key(fe.filter_hash,"xyzzy3",strlen("xyzzy3"),"hello3");
	hash_insert_buffer_key(fe.filter_hash,"xyzzy1",strlen("xyzzy1"),"hello3");
	hash_insert_string_key(fe.filter_hash,"mycal","hello3");
	hash_insert_string_key(fe.filter_hash,"mycal1","hello13");
	hash_insert_string_key(fe.filter_hash,"mycal2","hello23");
	hash_insert_string_key(fe.filter_hash,"mycal3","hello33");
	hash_insert_string_key(fe.filter_hash,"mycal4","hello43");
	hash_insert_buffer_key(fe.filter_hash,"11",strlen("11"),"hello3");
	hash_insert_buffer_key(fe.filter_hash,"12",strlen("12"),"hello3");
	hash_insert_buffer_key(fe.filter_hash,"13",strlen("13"),"hello3");
	hash_insert_buffer_key(fe.filter_hash,"14",strlen("14"),"hello3");
	hash_insert_buffer_key(fe.filter_hash,"15",strlen("15"),"hello3");
	hash_insert_buffer_key(fe.filter_hash,"16",strlen("16"),"hello3");
	hash_insert_buffer_key(fe.filter_hash,"aabcdefg",strlen("abcdefg"),"hello");
	hash_insert_buffer_key(fe.filter_hash,"axyzzy",strlen("axyzzy"),"hello1");
	hash_insert_buffer_key(fe.filter_hash,"axyzzy1",strlen("axyzzy1"),"hello2");
	hash_insert_buffer_key(fe.filter_hash,"axyzzy3",strlen("axyzzy3"),"hello3");
	hash_insert_buffer_key(fe.filter_hash,"axyzzy1",strlen("axyzzy1"),"hello3");
	hash_insert_buffer_key(fe.filter_hash,"amycal",strlen("amycal"),"hello3");
	hash_insert_string_key(fe.filter_hash,"99","helloa3");
	hash_insert_string_key(fe.filter_hash,"98","hello13");
	hash_insert_string_key(fe.filter_hash,"977","hello23");
	hash_insert_string_key(fe.filter_hash,"881","hellos33");
	hash_insert_string_key(fe.filter_hash,"mycal4","hello43");

	printf("lookup %s\n",(char*)yhash_lookup_string(fe.filter_hash,"xyzzy"));
	printf("lookup %s\n",(char*)yhash_lookup_string(fe.filter_hash,"xyzzy1"));
	printf("lookup %s\n",(char*)yhash_lookup_string(fe.filter_hash,"abcdefg"));
	printf("lookup %s\n",(char*)yhash_lookup_string(fe.filter_hash,"mycal"));
	printf("lookup %s\n",(char*)yhash_lookup_string(fe.filter_hash,"16"));

	printf("delete %s\n",(char*)yhash_delete_string(fe.filter_hash,"16"));

	printf("lookup %s\n",(char*)yhash_lookup_string(fe.filter_hash,"16"));
#endif

