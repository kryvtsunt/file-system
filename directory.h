#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_SIZE 64
#define DIR_NAME 48

#include "slist.h"


// this is a directory entry, very confusing I know
typedef struct dir_ent {
    char* file_name;
    int pnum; // inode idx
} dir_ent;

// this is a directory
typedef struct directory {
  char* dir_name;
  dir_ent entries[32];
  int number_of_entries;
} directory;

void directory_init(directory* dur, char* name);
int directory_entry_lookup(directory* dir, char* name);
int add_directory(directory* dir, char* name, int pnum);
int delete_directory(directory* dir, int entry_idx);

#endif

