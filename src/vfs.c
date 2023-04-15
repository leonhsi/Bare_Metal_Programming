#include "vfs.h"
#include "tmpfs.h"
#include "utils.h"
#include "uart.h"

struct mount* rootfs;
struct filesystem* fs_list_head;

int init_tmpfs(){
    rootfs = (struct mount *)simple_malloc(sizeof(struct mount));
    rootfs->fs = (struct filesystem *)simple_malloc(sizeof(struct filesystem));
    rootfs->fs->name = "tmpfs";
    rootfs->fs->setup_mount = tmpfs_setup_mount;
    rootfs->fs->next = NULL;

    rootfs->fs->setup_mount(rootfs->fs, rootfs);
    register_filesystem(rootfs->fs);

    return 1;
}

//check filesystem list when vfs_mount
//vfs_mount(char *name) : use name of filesystem to mount, it will take this name to find fs on fs_list
int register_filesystem(struct filesystem* fs) {
    // register the file system to the kernel.
    struct filesystem *tmp = fs_list_head;
    while(tmp != NULL){
        tmp = tmp->next;
    }
    tmp->next = fs;
    fs->next = NULL;
    return 1;
}

int vfs_mkdir(const char *pathname){
    struct vnode *vnode_itr = rootfs->root;
    struct vnode *next_node = (struct vnode *)simple_malloc(sizeof(struct vnode));

    char str[200];
    strcpy(str, pathname);
    char s[2] = "/";
    char *token;

    token = strtok(str, s);
    while(token != NULL){
        int ret = vnode_itr->v_ops->lookup(vnode_itr, &next_node, token);
        if(ret == -1) break;    //vnode_itr will be parent of dir 
        vnode_itr = next_node;
        token = strtok(NULL, s);
    }
    //create dir
    vnode_itr->v_ops->mkdir(vnode_itr, &next_node, token);
    return 1;
}

int vfs_mount(const char *target, const char *filesystem){
    //find target vnode to mount
    struct vnode *vnode_itr = rootfs->root;
    struct vnode *next_vnode = (struct vnode *)simple_malloc(sizeof(struct vnode));

    char str[200];
    strcpy(str, target);
    char s[2] = "/";
    char *token;

    token = strtok(str, s);
    while(token != NULL){
        int ret = vnode_itr->v_ops->lookup(vnode_itr, &next_vnode, token);
        if(ret != SUCCESS_FOUND_FILE) return 0;
        vnode_itr = next_vnode;
        token = strtok(NULL, s);
    }

    //find filesystem on fs_list
    struct filesystem *fs = fs_list_head;
    while(fs != NULL){
        fs = fs->next;
        if(fs->name == filesystem) break;
    }
    fs->setup_mount(fs, vnode_itr->mount);

    return 1;
}

int vfs_lookup(const char *pathname, struct vnode **target){
    struct vnode *vnode_itr = rootfs->root;

    char str[200];
    strcpy(str, pathname);
    char s[2] = "/";
    char *token;

    token = strtok(str, s);
    while(token != NULL){
        struct vnode *next_node = (struct vnode *)simple_malloc(sizeof(struct vnode));
        int ret = vnode_itr->v_ops->lookup(vnode_itr, &next_node, token);
        if(ret != 1) return ret;
        vnode_itr = next_node;
        token = strtok(NULL, s);
    }

    *target = vnode_itr;

    return 1;
}

int vfs_write(struct file* file, const void* buf, size_t len) {
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    struct vnode *vnode = file->vnode;
    int ret = vnode->f_ops->write(file, buf, len);
    if(ret == len) return 1;
    else return ERR_SIZE_FAULT;
}

int vfs_read(struct file* file, void* buf, size_t len) {
  // 1. read min(len, readable file data size) byte to buf from the opened file.
  // 2. return read size or error code if an error occurs.
  struct vnode *vnode = file->vnode;
  int ret = vnode->f_ops->read(file, buf, len);
  if(ret == len) return 1;
  else return ERR_SIZE_FAULT;
}

struct file* vfs_open(const char* pathname, int flags) {
    // 1. Lookup pathname from the root vnode.
    struct vnode *vnode_itr = rootfs->root;
    struct vnode *next_node = (struct vnode *)simple_malloc(sizeof(struct vnode));

    char str[200];
    strcpy(str, pathname);
    char s[2] = "/";
    char *token;

    token = strtok(str, s);
    while(token != NULL){
        //printf("token : %s\n", token);
        int ret = vnode_itr->v_ops->lookup(vnode_itr, &next_node, token);
        //printf("ret : %d\n", ret);
        if(ret == ERR_FILE_NOT_FOUND && (flags & O_CREAT)){
            vnode_itr->v_ops->create(vnode_itr, &next_node, token);
        }
        else if(ret == ERR_VNODE_NOT_DIR) break;
        vnode_itr = next_node;
        //printf("vnode_itr->name : %s\n", ((struct tmpfs_data *)(vnode_itr->internal))->name );
        //printf("next_node->name : %s\n", ((struct tmpfs_data *)(next_node->internal))->name );
        token = strtok(NULL, s);
    }

    // 2. Create a new file descriptor for this vnode if found.
    struct file *fd = (struct file *)simple_malloc(sizeof(struct file));
    int ret = vnode_itr->f_ops->open(vnode_itr, &fd);
    //printf("vnode->name : %s\n", ((struct tmpfs_data *)(vnode_itr->internal))->name );
    //printf("fd->name : %s\n", ((struct tmpfs_data *)(fd->vnode->internal))->name );
    if(ret == -1) return NULL;

    // 3. Create a new file if O_CREAT is specified in flags.

    return fd;
}

int vfs_close(struct file* file) {
  // 1. release the file descriptor
  struct vnode *vnode = file->vnode;
  vnode->f_ops->close(file);
  return 1;
}