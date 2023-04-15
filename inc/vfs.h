#ifndef VFS__H
#define VFS__H

#include "stdint.h"
#include "list.h"

#define O_CREAT 00000100

struct vnode{
  struct mount* mount;  //mount point
  struct vnode_operations* v_ops;   //point to tmpfs
  struct file_operations* f_ops;
  void* internal;   //point to itself's data structure

  //reference count ? 如果vnode dynamic產生 沒人在用就可以free掉 則需要reference count
};

//file descriptor
struct file {
  struct vnode* vnode;
  size_t f_pos; // The next read/write position of this opened file
  struct file_operations* f_ops;
  int flags;
};

struct mount {
  struct vnode* root;
  struct filesystem* fs;
};

struct filesystem {
  char* name;
  int (*setup_mount)(struct filesystem* fs, struct mount* mount);
  struct filesystem *next;
};

struct file_operations {
  int (*write) (struct file* file, const void* buf, size_t len);    
  int (*read) (struct file* file, void* buf, size_t len);
  int (*open) (struct vnode* file_node, struct file **target);       //open a file for this vnode
  int (*close) (struct file* file);
};

struct vnode_operations {
  int (*lookup)(struct vnode* dir_node, struct vnode** target, const char* component_name);
  int (*create)(struct vnode* dir_node, struct vnode** target, const char* component_name);
  int (*mkdir)(struct vnode* dir_node, struct vnode** target, const char* component_name);
};

struct fs_list{
    struct list_head *head;
};

extern struct filesystem *fs_list;
extern struct mount* rootfs;

int init_tmpfs();
int register_filesystem(struct filesystem* fs);
struct file* vfs_open(const char* pathname, int flags);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, size_t len);
int vfs_read(struct file* file, void* buf, size_t len);
int vfs_mkdir(const char *pathname);
int vfs_mount(const char *target, const char *filesystem);

#endif