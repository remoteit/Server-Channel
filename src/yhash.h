#ifndef __YHASH_H__
#define __YHASH_H__
/*! \file yhash.h
    \brief yoics hash
*/

#define YHASH_STATISTICS 0
//
// Optimize for particular app, this is the starting table size or the fixed size if no resizing, don't make to big or malloc might break, best preformance when under
//  60% utilization
//
#define	YHASH_DEFAULT_LOOKUP_TABLE_SIZE		8	// As in 2^10 (or 4096)
//
// Automatically Resize when hash gets X % full
#define YHASH_AUTO_RESIZING					0

// hash type
enum 
{ 
    YHASH_STANDARD=0, 
    YHASH_MAP,
};       

typedef struct yhash_element_
{
	struct yhash_element_	*next;
	int			hash_index;				// Current Hash Index (needed?)
	char		*key;					// key
	int			key_len;				// keylen
	void		*data;					// Linked Data
}YHASH_OBJ;

typedef struct yhash_
{
    long        type;           // type of hash, standard data, or string map (string map handles alloc and dealloc and copies data string)
	long		elements;		// number of elements in hash
	long		mask;			// Current Mask
	long		hash_size;		// This is X in the 2^X size
#if defined(YHASH_STATISTICS)
	// Statistics
	long		inserts;
	long		insert_chains;	
	long		deletes;
	long		lookups;
	long		lookup_fails;
#endif
	// Array of pointers
	YHASH_OBJ	*hash[];		// Pointer to current hash array
}YHASH;



//Public
YHASH *yhash_init(int type, int size);
int yhash_destroy(YHASH *hash, void (*callback)(void *));
int yhash_insert_buffer_key(YHASH *hash, char *key, int keylen, void *value);
int yhash_insert_string_key(YHASH *hash, char *key, void *value);
void *yhash_lookup_buffer(YHASH *hash, char *key, int keylen);
void *yhash_lookup_string(YHASH *hash, char *key);
void *yhash_delete_buffer(YHASH *hash, char *key, int keylen);
void *yhash_delete_string(YHASH *hash, char *key);
void yhash_print_stats(YHASH *hash);
// Private
YHASH_OBJ *yhash_lookup_object(YHASH *hash, char *key, int keylen);

#endif
