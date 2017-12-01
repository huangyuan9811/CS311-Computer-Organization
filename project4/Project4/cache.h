#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* cache.h : Declare functions and data necessary for your project*/

#define TAG_BIT(ADDR) (((uintptr_t) ADDR) >> 4)
#define INDEX_BIT(ADDR) ((((uintptr_t)ADDR) >> 3) & 0x1)
#define WORD_BIT(ADDR) (((uintptr_t)ADDR & 0x7) >>2)

#define VALID_BIT(ADDR) (((uintptr_t) ADDR) >> 31)
#define DIRTY_BIT(ADDR) ((((uintptr_t) ADDR) <<1) >> 31)
#define CACHE_BLOCK_TAG_BIT(VAL) ((((uint32_t) VAL) << 2) >> 2)

int miss_penalty; // number of cycles to stall when a cache miss occurs
uint32_t ***Cache; // data cache storing data [set][way][byte]
uint32_t ***Cache_helper;

void setupCache(int, int, int);
void setCacheMissPenalty(int);

uint32_t is_data_in_cache(uint32_t address, bool is_store_word, uint32_t data);
uint32_t load_data_into_cache(uint32_t address, bool is_store_word, uint32_t data);
uint32_t insert_into_cache(uint32_t address);
int set_0_queue [4];
int set_1_queue [4];
