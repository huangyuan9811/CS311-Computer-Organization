#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* cache.h : Declare functions and data necessary for your project*/

#define TAG_BIT(ADDR) (((uintptr_t) ADDR) >> 4)
#define INDEX_BIT(ADDR) ((((uintptr_t)ADDR) >> 3) & 0x1f)

#define VALID_BIT(ADDR) (((uintptr_t) ADDR) >> 31)
#define DIRTY_BIT(ADDR) ((((uintptr_t) ADDR) <<1) >> 31)
#define CACHE_BLOCK_TAG_BIT(ADDR) ((((uintptr_t) ADDR) << 2) >> 1)

#define CACHE_BLOCK_FIRST_4(ADDR) (((uintptr_t) ADDR) >> 32)
#define CACHE_BLOCK_LAST_4(ADDR) ((((uintptr_t) ADDR) << 32) >> 32)

int miss_penalty; // number of cycles to stall when a cache miss occurs
uint32_t ***Cache; // data cache storing data [set][way][byte]

void setupCache(int, int, int);
void setCacheMissPenalty(int);

uint32_t is_data_in_cache(uint32_t address);
uint32_t load_data_into_cache(uint32_t address);
uint32_t insert_into_cache(uint32_t address);
int set_0_queue [4];
int set_1_queue [4];
