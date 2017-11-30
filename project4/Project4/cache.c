#include "cache.h"
#include "util.h"

/* cache.c : Implement your functions declared in cache.h */

void shift_queue (int queue[], int recent_index);
void update_cache_block(uint32_t * updating_block, uint32_t tagbit, uint32_t data);

/***************************************************************/
/*                                                             */
/* Procedure: setupCache                  		       */
/*                                                             */
/* Purpose: Allocates memory for your cache                    */
/*                                                             */
/***************************************************************/

void setupCache(int capacity, int num_way, int block_size)
{
/*	code for initializing and setting up your cache	*/
/*	You may add additional code if you need to	*/
	/*
	capacity = 64 bytes
	num_way = 4
	block_size = 8 bytes
	*/
	int i,j; //counter
	int nset=0; // number of sets
	int _wpb=0; //words per block   
	nset=capacity/(block_size*num_way); // 2 sets
	_wpb = block_size/BYTES_PER_WORD; // 2 words per block (BYTES_PER_WORD = 4)

	Cache = (uint32_t  ***)malloc(nset*sizeof(uint32_t **));
	
	for (i=0;i<nset;i++) {
		Cache[i] = (uint32_t ** )malloc(num_way*sizeof(uint32_t*));
	}
	for (i=0; i<nset; i++){	
		for (j=0; j<num_way; j++){
			Cache[i][j]=(uint32_t*)malloc(sizeof(uint32_t)*(_wpb));
		}
	}

}


/***************************************************************/
/*                                                             */
/* Procedure: setCacheMissPenalty                  	       */
/*                                                             */
/* Purpose: Sets how many cycles your pipline will stall       */
/*                                                             */
/***************************************************************/

void setCacheMissPenalty(int penalty_cycles)
{
/*	code for setting up miss penaly			*/
/*	You may add additional code if you need to	*/	
	miss_penalty = penalty_cycles;
}

/* Please declare and implement additional functions for your cache */

/* Tag - index - offset bit calculation
 * 1 block == 2^3 bytes ==> 3 bits for offset 
 * only 2 sets ==> 1 bits for index  INDEX_BIT
 * 32 - 3 - 2 == 27 ==> 28 bits for tag ? TAG_BIT
 */

uint32_t is_data_in_cache(uint32_t address){
	int indexbit = (int)INDEX_BIT(address);
	uint32_t tagbit = TAG_BIT(address);
	uint32_t* cached_block;
	bool valid = 0;
	bool is_tag_matching = 0;
	bool cache_hit = 0;
	uint32_t cached_data;
	int cache_hit_spot = -1;

	// search if data is in the cache
	int i;
	for (i = 0; i < 4; i++) {
		// Cache[indexbit][i] is 8 bytes of data == 1 block 
		cached_block = Cache[indexbit][i];
		valid = (bool)VALID_BIT(CACHE_BLOCK_FIRST_4(cached_block));
		is_tag_matching = ~(tagbit ^ (CACHE_BLOCK_TAG_BIT(CACHE_BLOCK_FIRST_4(cached_block))));
		if (valid & is_tag_matching) {
	 		cache_hit = 1;
			cache_hit_spot = i;
			break;
		}
	}
	
	// if cache hit, return the data right away
	if (cache_hit) {
		cached_data = CACHE_BLOCK_LAST_4(cached_block);
	    // shift queue since accessed	
		if (indexbit == 0)
			shift_queue(set_0_queue, cache_hit_spot);
		else if(indexbit == 1)
			shift_queue(set_1_queue, cache_hit_spot);
		return cached_data;
	}
	return 0; // what if the data is 0...omg whatever....
}

uint32_t load_data_into_cache(uint32_t address){
	int indexbit = (int) INDEX_BIT(address);
	uint32_t tagbit = TAG_BIT(address);
	uint32_t *cached_block;
	uint32_t read_data;
	bool valid = 0;
	int free_spot = -1;
	uint32_t *victim_block;
	bool dirtybit = 0;

	// check if there is a free spot
	int i;
	for (i =0;i < 4;i++){
		cached_block = Cache[indexbit][i];
		valid = (bool) VALID_BIT(CACHE_BLOCK_FIRST_4(cached_block));
		if (valid == 0) {
			free_spot = i;
			break;
		} 
	}
	
	// free spot found! load data into the cache
	if (free_spot > -1) {
		read_data = mem_read_32(address);
		update_cache_block(cached_block, tagbit, read_data);
		// since it has been accessed, shift queue 
		if (indexbit == 0)
			shift_queue(set_0_queue, free_spot);
		else if(indexbit == 1)
			shift_queue(set_1_queue, free_spot);
	}
	else { // select a victim since there is no free spot 
		int victim_num;
		if (indexbit == 0) {
			victim_num = set_0_queue[0];
			victim_block = Cache[indexbit][victim_num];
			dirtybit = (bool) DIRTY_BIT(CACHE_BLOCK_FIRST_4(victim_block));
			if (dirtybit) {
				// write back
				mem_write_32(address, CACHE_BLOCK_LAST_4(victim_block));				
			}
			// load new data block
			read_data = mem_read_32(address);
			update_cache_block(victim_block, tagbit, read_data);
			shift_queue(set_0_queue, victim_num);
		} else {
			victim_num = set_1_queue[0];
			victim_block = Cache[indexbit][victim_num];
			dirtybit = (bool) DIRTY_BIT(CACHE_BLOCK_FIRST_4(victim_block));
			if (dirtybit) {
				// write back
				mem_write_32(address, CACHE_BLOCK_LAST_4(victim_block));				
			}
			// load new data block
			read_data = mem_read_32(address);
			update_cache_block(victim_block, tagbit, read_data);
			shift_queue(set_1_queue, victim_num);
		}
	}
	return read_data;
}

uint32_t insert_into_cache(uint32_t address) {
	// go through the Cache in specific slot according to the address
	// if there is no free space, make it and insert the data into cache
	return 0;		
}

/* shift the queue when cache is accessed for LRU implementation */
void shift_queue (int queue[], int recent_index) {
	int tail = recent_index;
	bool located = 0;
	int i;
	for (i = 0; i < 3; i++) {
		if (queue[i] == recent_index || located == 1) {
			queue[i] = queue[i+1];
			located = 1;
		}
	}
	queue[3] = tail;
}

/* update the cache block when eviction happens */
void update_cache_block(uint32_t * updating_block, uint32_t tagbit, uint32_t data){
	updating_block[0] = tagbit | 0x80000000;
	updating_block[1] = data;
}
