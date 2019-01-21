#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <bsd/string.h>
#include <assert.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "storage.h"
#include "memory.h"
#include "util.h"
#include "data_block.h"
#include "inode.h"
#include "slist.h"
#include "directory.h"

// implementation for: man 2 access
// Checks if a file exists.

int
nufs_access(const char *path, int mask)
{
    printf("access(%s, %04o)\n", path, mask);

    // retrive index of node give nthe path
    int idx = get_entry_index(path);
    if (idx < 0) {
      return -ENOENT; // NEED TO RETURN THIS ERROR
    }
  
    // path was found so return 0 I guess
    return 0;
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
{
    printf("getattr(%s)\n", path);
    // send to storage to set up stat struct's fields
    int rv = get_stat(path, st);
    if (rv == -1) {
        return -ENOENT;
    }
    else {
      return rv; // THIS HAS TO BE RETURNED APPARENTLY
    }
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    // make a stat struct
    struct stat st;

    printf("readdir(%s)\n", path);

    // get index of block for given path
    int idx = get_entry_index(path);
    if (idx < 0) {
      return -ENOENT; // explode if it caanot find an index, becuase this should never happed
    }

    // drill to get the directory releveant to this index
    directory* cur_dir = get_data_block_addr(idx);

    // fill in the info for all things in this directory
    for (int ii = 0; ii < 32; ii++) {
      dir_ent cur_ent = cur_dir->entries[ii];

      if (cur_ent.file_name == NULL) { // if for wharever reason
      }
      else {
	filler(buf, cur_ent.file_name, NULL, 0);
      }
    }

    return 0;


    /*
    get_stat("/", &st);
    // filler is a callback that adds one item to the result
    // it will return non-zero when the buffer is full
    filler(buf, ".", &st, 0);

    get_stat("/hello.txt", &st);
    filler(buf, "hello.txt", &st, 0);

    return 0;
    */
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    printf("mknod(%s, %04o)\n", path, mode);

    // get index of block with given path
    int idx = get_entry_index(path);
    if (idx >= 0) {
      return -1; // explode if cannot be found, which SHOULD NEVER HAPPED
    }

    // get next available index   
    int aval_idx = get_next_free_inode_idx(inode_bitmap_addr());
    if (aval_idx < 0) {
      return aval_idx; // return error if index is not valid
    }

    // get the inode, initialize it, and update the bitmaps 
    inode* cur_inode = get_inode_address(aval_idx);
    inode_create(cur_inode, mode, 1, 0);
    inode_bitmap_addr()[aval_idx] = 1;
    get_data_block_bitmap_address()[aval_idx] = 1;
    
    // add new entity to appropriate directory
    int new_entry_idx = add_dir_entry(path, aval_idx);
    if (new_entry_idx < 0) {
      return new_entry_idx;
    }

    return 0;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
    printf("mkdir(%s)\n", path);

    // we got to check if such a path exists
    int index = get_entry_index(path);
    if (index >= 0) { // this mean a path alread exists and an error should be thrown
      return -1;
    }

    // get the next available index to use for the directory
    int aval_idx = get_next_free_inode_idx(inode_bitmap_addr());
    if (aval_idx < 0) {
      return aval_idx; // make sure valid index is found
    }

    // set up the inode    
    inode *cur_inode = get_inode_address(aval_idx);
    inode_create(cur_inode, mode, 1, 0);
    inode_bitmap_addr()[aval_idx] = 1;

    // set up the directory 
    directory *cur_dir = get_data_block_addr(aval_idx);
    directory_init(cur_dir, slist_last(path)->data);
    get_data_block_address()[aval_idx] = cur_dir;
    get_data_block_bitmap_address()[aval_idx] = 1;

    // add the directory to the system
    int new_entry_index = add_dir_entry(path, aval_idx);
    if (new_entry_index < 0) {
      return new_entry_index;
    }

    return -1;
}

int
nufs_unlink(const char *path)
{
    printf("unlink(%s)\n", path);
    
/*   int idx = get_entry_index(path);

    int cur_inode = get_inode_address(idx);
   
    cur_inode->user = 0;
    cur_inode->mode = 0;
    cur_inode->type = 0;
    cur_inode->size = 0;
   


    data_block_bitmap_addr()[idx] = 0;
    inode_bitmap_addr()[idx] = 0;

    directory* cur_dir = get_data_block_address(idx);
    int dir_idx = directory_entry_lookup(cur_dir, path);
    int foo = directory_delete_entry(cur_dir, dir_idx);

*/
    return -1;
    
}

int
nufs_rmdir(const char *path)
{

    printf("rmdir(%s)\n", path);

    int idx = get_entry_index(path);
    if (idx >= 0) {
	return -1; // something went wrong
    }

    inode* cur_inode = get_inode_address(idx);
    // clean up inode for reuse

    directory* cur_dir = get_data_block_addr(idx);
    // clean up and remove directory

    // reset bitmap values
    inode_bitmap_addr()[idx] = 0;
    get_data_block_bitmap_address()[idx] = 0;


    return -1;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
    printf("rename(%s => %s)\n", from, to);

    return -1;
}

int
nufs_chmod(const char *path, mode_t mode)
{
    printf("chmod(%s, %04o)\n", path, mode);
    // not implemented

    return -1;
}

int
nufs_truncate(const char *path, off_t size)
{
    printf("truncate(%s, %ld bytes)\n", path, size);

    int idx = get_entry_index(path);
    if (idx < 0) {
	return -1;
    }
    // set inode size to new size
    inode* node = get_inode_address(idx);
   
    node->size = size;

    return -1;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
    printf("open(%s)\n", path);
    // actually we have found you don't have to do anything... or at least we think   
    return 0;

}
// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    printf("read(%s, %ld bytes, @%ld)\n", path, size, offset);

    const char* data = get_data(path);


    // read and copry data over to buffer
    int len = strlen(data) + 1;
    if (size < len) {

        len = size;
    }
    
    strlcpy(buf, data, len);
    return len;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    printf("write(%s, %ld bytes, @%ld)\n", path, size, offset);
    
    
    // some simple erroy checking path exists and size is supported
    int idx = get_entry_index(path);
    if (idx < 0) {
      return -1;
    }

    data_block *cur_block = get_data_block_addr(idx);
    if (offset + size > 4096) {
      return -1;
    }

    // copy budder to desired location
    memcpy(cur_block + offset, buf, size);
    
    inode* cur_inode = get_inode_address(idx);
    cur_inode->size = size;

    return size;
}

// Update the timestamps on a file or directory.
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    //int rv = storage_set_time(path, ts);
    int rv = -1;
    printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
           path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
// not implemented
	return rv;
}

void
nufs_init_ops(struct fuse_operations* ops)
{
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access   = nufs_access;
    ops->getattr  = nufs_getattr;
    ops->readdir  = nufs_readdir;
    ops->mknod    = nufs_mknod;
    ops->mkdir    = nufs_mkdir;
    ops->unlink   = nufs_unlink;
    ops->rmdir    = nufs_rmdir;
    ops->rename   = nufs_rename;
    ops->chmod    = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open	  = nufs_open;
    ops->read     = nufs_read;
    ops->write    = nufs_write;
    ops->utimens  = nufs_utimens;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
    assert(argc > 2 && argc < 6);
    storage_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}

