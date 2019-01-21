#ifndef MEMORY_H
#define MEMORY_H


#include <stdio.h>

#include "storage.h"

// struct for 'memory' of whole disk. Should be placed at front of disk
typedef struct memory {
  // offset location of inode bitmap starting point
  size_t inode_bitmap_start;
  // offset location of data_block bitmap starting point
  size_t data_block_bitmap_start;
  // offset location where inods begin
  size_t inode_start;
  // offset location where data_blocks begin
  size_t data_block_start;
  // location of inode for the root folder. Has proven very convenient
  int root_inode_index;
} memory;


void memory_setup(void* disk);
void memory_free();
void memory_add_node(const char* path);
memory* memory_addr();

#endif
