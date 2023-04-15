#ifndef TMPFS__H
#define TMPFS__H

#include "stdint.h"
#include "vfs.h"

//tmpfs lookup code
#define SUCCESS_FOUND_FILE 1
#define ERR_FILE_NOT_FOUND -1
#define ERR_VNODE_NOT_DIR 0

//tmpfs create code
#define SUCCESS_CREATE_FILE 1
#define ERR_FILE_EXIST -1

//tmpfs file op code
#define ERR_SIZE_FAULT -1

struct tmpfs_data{
    char* type;
    uint64_t size;  //read or write should check it
    char *name;
    void *data;     //point to dirdata or filedata
    struct vnode *node;
};

struct tmpfs_dirdata{
    struct{
        char name[16];
        struct tmpfs_data *next;
    }entry[16];
};

struct tmpfs_filedata{
    void *data;
};

//tmpfs_setup_mount
int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount);

//tmpfs_fop
int tmpfs_write(struct file* file, const void* buf, size_t len);    //point to tmpfs_write
int tmpfs_read(struct file* file, void* buf, size_t len);
int tmpfs_open(struct vnode* file_node, struct file **target);
int tmpfs_close(struct file *file);

//tmpfs_vop
int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name);
int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name);

struct tmpfs_data *tmpfs_new_data(char *type, const char *name, struct vnode *self, int size);
struct vnode *tmpfs_new_vnode(struct tmpfs_data *td);
struct tmpfs_dirdata *tmpfs_new_dirdata();
struct tmpfs_filedata *tmpfs_new_filedata();
struct tmpfs_data *tmpfs_create_file(char *type, const char *name);

//initramfs
void mount_initramfs();
void open_initramfs_file(char *addr, char *dir, char *filename, int ns, int fs);

#endif