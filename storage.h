#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


void storage_init(const char* path);
int  get_stat(const char* path, struct stat* st);
const char* get_data(const char* path);
int get_entry_index(char* path);
int add_dir_entry(char* path, int new_inode_idx);
int remove_dir_entry(char* path);
void* get_disk();


#endif
