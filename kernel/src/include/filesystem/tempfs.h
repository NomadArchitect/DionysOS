//
// Created by dustyn on 9/18/24.
//

#pragma once
#include "include/types.h"
#include "include/filesystem/vfs.h"

#define TEMPFS_BLOCKSIZE 1024
#define TEMPFS_MAGIC 0x7777777777777777
#define TEMPFS_VERSION 1
#define MAX_LEVEL_INDIRECTIONS 5
#define MAX_FILENAME_LENGTH 128 /* This number is here so we can fit 2 inodes in 1 2048 block */
//marks this block as an indirection block , an array of 64bit block pointers
#define INDRECTION_HEADER 0x123456789ABCEFEC


extern struct vnode_operations tempfs_vnode_ops;

struct tempfs_superblock {
  uint64 magic;
  uint64 version;
  uint64 block_size;
  uint64 num_blocks;
  uint64 num_inodes;
  uint64 total_size;
  /* Both bitmap entries hold n block pointers */
  uint64 inode_bitmap_pointers[6]; /* Can hold 50k inodes assuming 1024 size */
  uint64 block_bitmap_pointers[114]; /*  1mib worth of blocks */
  uint64 inode_start_pointer; /* Where inodes start */
  uint64 block_start_pointer; /* Where blocks start */
};

//4 inodes per block
struct tempfs_inode {
  uint16 uid;
  uint16 gid;
  uint16 type;
  uint16 refcount;
  char name[MAX_FILENAME_LENGTH];
  uint64 size;
  uint64 blocks[13]; /* Will point to logical block numbers */
  uint64 reserved;
};

struct tempfs_block {
  uint64 data[sizeof(uint64) * (TEMPFS_BLOCKSIZE / sizeof(uint64))];
};

void tempfs_init();
uint64 tempfs_read(struct vnode *vnode,uint64 offset, char *buffer, uint64 bytes);
uint64 tempfs_write(struct vnode *vnode,uint64 offset, char *buffer, uint64 bytes);
uint64 tempfs_stat(struct vnode *vnode,uint64 offset, char *buffer, uint64 bytes);
struct vnode *tempfs_lookup(struct vnode *vnode, char *name);
struct vnode *tempfs_create(struct vnode *vnode,struct vnode *new_vnode,uint8 vnode_type);
void tempfs_close(struct vnode *vnode);
struct vnode *tempfs_link(struct vnode *vnode,struct vnode *new_vnode);
void tempfs_unlink(struct vnode *vnode);
void tempfs_remove(struct vnode *vnode);
void tempfs_rename(struct vnode *vnode,char *new_name);
uint64 tempfs_open(struct vnode *vnode);

