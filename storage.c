
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include "directory.h"
#include "memory.h"
#include "inode.h"
#include "util.h"
#include "data_block.h"
#include "storage.h"
#include "slist.h"


typedef struct file_data {
    const char* path;
    int         mode;
    const char* data;
} file_data;

static file_data file_table[] = {
};


static const SIZE_OF_DISK = 1048576;
void* disk = 0;


static memory* mem;

  // sets up offset fields at front of disk
memory_setup(void* disk) {
  mem = disk;

  // inode bitmap offset
  mem->inode_bitmap_start = disk + sizeof(memory);

  // data bitmap offset
  mem->data_block_bitmap_start = mem->inode_bitmap_start + 256;

  // inode offset
  mem->inode_start = mem->data_block_bitmap_start + 256;

  // data offset
  mem->data_block_start = mem->inode_start + 256 * sizeof(inode);

  // offset pointers are set
}

// getter to front of disk
memory*
memory_addr() {
  return mem;
}

// set up the fields of the given inode
void inode_create(inode* inode, mode_t mode, int type, size_t size) {

  inode->user = getuid();
  inode->mode = mode;
  inode->type = type;
  inode->size = size;

}

// returns index of next free inode
int get_next_free_inode_idx(int* inode_bitmap) {
  for (int ii = 1; ii < 256; ii++) {
    if (inode_bitmap[ii] == 0) {
      return ii;
     
    }
  }

  return -1;

}

// returns inode address
inode* inodes_addr() {
  return (inode*) (memory_addr()->inode_start);
}

// returns address in bitmap
int* inode_bitmap_addr() {
  return (int*) (memory_addr()->data_block_bitmap_start);
}

// returns exact location of inode on disk of given index
void* get_inode_address(int index) {
  return (void*) (inodes_addr() + sizeof(inode) * index);
}


// get the next bitmap index for a free data block
int get_next_free_data_block_idx(int* data_block_bitmap) {
  for (int ii = 1; ii < 64 ; ++ii) {
    if (data_block_bitmap[ii] = 0) {
      return ii;
    }
  }
  return -1;

}

// getter function gets address where data blocks begin
void** get_data_block_address() {
  return (void**) memory_addr()->data_block_start;
}

// getter function gets address where data block bitmap begins
int* get_data_block_bitmap_address() {
  return (int*) memory_addr()->data_block_bitmap_start;
}

// gets EXACT address of data block, moving to where data blcoks
// begin, then moving the givin index over to the data block
void* get_data_block_addr(int index) {
  return (void*) (get_data_block_address() + 4096 * index);
}


// initallize the directory
void directory_init(directory* dir, char* name) {
  if (sizeof(name) > 28) {
    // does not support directories of larger size
    return;
  }
  // sets fields
  dir->dir_name = name;
  dir->number_of_entries = 0;
}

// returns index which corresponds the the directory location
// on disk with the given name
int directory_entry_lookup(directory* dir, char *name) {
  int num_of_entries = dir->number_of_entries;
  for (int ii = 0; ii < 32; ii++) {
    dir_ent cur_entry = dir->entries[ii];
    if (cur_entry.file_name == NULL) {
      continue;
    }
    if (streq(cur_entry.file_name, name)) {
      return ii;
    }

  }
  return -ENOENT;
}

// add a directory to the system
int add_directory(directory *dir, char *name, int inode_idx) {
  int num_of_entries = dir->number_of_entries;
  
  if (num_of_entries >= 32 || sizeof(*name) > 27) {
    return -1;
  }

  dir_ent de;
  de.file_name = name;
  de.pnum = inode_idx;
  
  for (int ii = 0; ii < 32; ii++) {
    dir_ent cur_entry = dir->entries[ii];
    if (cur_entry.file_name == NULL) {
      dir->entries[ii] = de;
      dir->number_of_entries++;
      return 0;
    }
  }
  return -1;
}


int delete_directory(directory* dir, int entry_idx) {
  int num_of_entries = dir->number_of_entries;
  if (num_of_entries <= 0) {
    return -ENOENT;
  }
  dir_ent cur_entry = dir->entries[entry_idx];
  memset(cur_entry.file_name, 0, 27);
  dir->number_of_entries--;

  return 0;
}


// ** This sets up the whole disk for the file system, ASSUMING
// IT HAS NOT BEEN SET UP ALREADY
void
storage_init(const char* path)
{
    printf("TODO: Store file system data in: %s\n", path);

    // open disk
    int fd;
    fd = open(path, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
      printf("could not open file\n");
      return;
    }

    // truncate to 1MB
    int rv = ftruncate(fd, SIZE_OF_DISK);
    char buffer[1024 * 1024];
    size_t reading = read(fd, &buffer, 1024 * 1024);


    assert(rv == 0);

    disk = mmap(0, SIZE_OF_DISK, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // NOTE: check if mmap failed


    // IMPORTANT: if if disk is NOT all null bytes, it has been made
    // already, set up the offsets again so the superblock sets up its
    // getter, then RETURN
    for (int qq = 0; qq < 1024 * 1024; qq++) {
	char* ptr = disk + qq;
	if (*ptr != '\0') {
	   printf("disk already exists!");
	   memory_setup(disk);
	   return;
	}
    }
	

    memory_setup(disk);
    // now memory is set up with offsets
   
    // set bitmaps all to zeros
    for (int ii = 0; ii < 256; ii++) {
      inode_bitmap_addr()[ii] = 0;
      get_data_block_bitmap_address()[ii] = 0;
    }

    memory* mem_addr = memory_addr();

    // have to setup root directroy
    int root_dir_idx = mem_addr->root_inode_index;
    inode *root_inode = get_inode_address(root_dir_idx);
    inode_create(root_inode, 040755, 0, 4096); // dirs are size
    // 4096 by default, and the 040755 is the standard coding
    // for the directory
    inode_bitmap_addr()[root_dir_idx] = 1;
    directory* root = get_data_block_addr(root_dir_idx);
    char* root_dir_name = "/"; // the name of the root
    directory_init(root, root_dir_name);
    get_data_block_bitmap_address()[root_dir_idx] = 1;

}


/*
static file_data*
get_file_data(const char* path) {
    for (int ii = 0; 1; ++ii) {
        file_data row = file_table[ii];

        if (file_table[ii].path == 0) {
            break;
        }

        if (streq(path, file_table[ii].path)) {
            return &(file_table[ii]);
        }
    }

    return 0;
}
*/


// very important function. It gets the index of the inode/data_block
// that was assigned the particular path
int get_entry_index(char* path) {

  int current_inode_idx = memory_addr()->root_inode_index;
  directory *root = get_data_block_addr(current_inode_idx);
  if (streq(path, root->dir_name)) {
    return 0; 
  }
  // utilzes s list to parse directory path
  slist* path_list = s_split(path, '/');
  if (streq(path_list->data, "")) {
    path_list = path_list->next;
  }
  directory* current = root;
  while (path_list != NULL) { // iterate throug slist
    int entry_idx = directory_entry_lookup(current, path_list->data);
    if (entry_idx < 0) {
      return -ENOENT;
    }
    // dir entity at the particular index
    dir_ent cur_ent = current->entries[entry_idx];
    int entry_inode_index = cur_ent.pnum;
    if (path_list->next == NULL) {
      return cur_ent.pnum;
    }
    else {
      current = get_data_block_addr(entry_inode_index);
      path_list = path_list->next;
    }
  }
  return -ENOENT;
}

// adds directory entry
int add_dir_entry(char* path, int new_inode_idx) {
  // use slist to parse path
  slist* path_list = s_split(path, '/');
  // gotta check if path is empty
  if (streq(path_list->data, "")) {
    path_list = path_list->next;
  }
  directory* root = get_data_block_addr(memory_addr()->root_inode_index);
  // start at the root
  directory *current = root;
  while (path_list != NULL) {
    int entry_idx = directory_entry_lookup(current, path_list->data);
    if ((entry_idx < 0) && (path_list->next == NULL)) {

    int new_entry_idx = add_directory(current, path_list->data, new_inode_idx);
      return new_entry_idx;
    }
    else {
      if (entry_idx < 0) {

	return -ENOENT;
      }
      //dir_ent cur_ent = current->entries[entry_idx];
      //int entry_inode_idx = cur_ent.pnum;
      //current = get_data_block_address(entry_inode_id)
    }
  }

  return -1;
}

int remove_dir_entry(char* path) {

  // use slist to parse path
  slist* path_list = s_split(path, '/');
  directory* root  = get_data_block_addr(memory_addr()->root_inode_index);
  if (streq(path_list->data, root->dir_name)) {
    path_list = path_list->next;
  }
  // start at the root directory
  directory *current = get_data_block_addr(memory_addr()->root_inode_index);
  while (path_list != NULL) {
    int entry_idx = directory_entry_lookup(current, path_list->data);
      if (entry_idx < 0) {
      return -ENOENT;
    }
    
    if (path_list->next == NULL) {
      delete_directory(current, entry_idx);
      return 1; // OR return resulst of directory_del_entry
    }
    else {
      dir_ent cur_ent = current->entries[entry_idx];
      int entry_inode_idx = cur_ent.pnum;
      current = get_data_block_addr(entry_inode_idx);
      path_list = path_list->next;
    }
  }
  return -1;
}


int
get_stat(const char* path, struct stat* st)
{
  // get index of path
  int idx = get_entry_index(path);
  if (idx < 0) {
    return -ENOENT;
  }

  memset(st, 0, sizeof(struct stat));
  // map inode stats to stat struct
  inode* cur_inode = get_inode_address(idx);
  st->st_uid = cur_inode->user;
  st->st_mode = cur_inode->mode;

  st->st_size = cur_inode->size;
  return 0;

  /*
    file_data* dat = get_file_data(path);
    if (!dat) {
        return -1;
    }

    memset(st, 0, sizeof(struct stat));
    st->st_uid  = getuid();
    st->st_mode = dat->mode;
    if (dat->data) {
        st->st_size = strlen(dat->data);
    }
    else {
        st->st_size = 0;
    }
    return 0;
  */
}

const char*
get_data(const char* path)
{
  // get index of path
  int idx = get_entry_index(path);
  if (idx < 0) {
    return -ENOENT;
  }
  // get inode and check that it is a file and NOT a directory
  inode* cur_inode = get_inode_address(idx);

  if (cur_inode->type == 1) {
    data_block* cur_block = get_data_block_addr(idx);
    // return the contents component of the datablock
    return cur_block->contents;
  }
  // otherwise return the directory, because what else would you do
  directory* cur_dir = get_data_block_addr(idx);
  return cur_dir;
}

// getter function
void* get_disk() {
  return disk;
}
