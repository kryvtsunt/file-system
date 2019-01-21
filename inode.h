#ifndef INODE_H
#define INODE_H


#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

// stucture of for inode, mood, user, size, type
typedef struct inode {

  mode_t mode;
  uid_t user; // TODO: ??
  size_t size;
  int type; // dir = 0, file = 1

} inode;

void inode_create(inode* inode, mode_t mode, int type, size_t size);
int get_next_free_inode_idx(int* inode_bitmap);
inode* inodes_addr();
int* inode_bitmap_addr();
void* get_inode_address(int idx);

#endif
