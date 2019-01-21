#ifndef DATA_BLOCK_H
#define DATA_BLOCK_H

#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

// sturct for data block. Just a 4k array of chars
typedef struct data_block {

  char contents[4096];

} data_block;

int get_next_free_data_block_idx(int* data_block_bitmap);
void** get_data_block_address();
int* get_data_block_bitmap_address();
void* get_data_block_addr(int idx);

#endif

