#include "tmpfs.h"
#include "utils.h"
#include "uart.h"
#include "initrd.h"

/*tmpfs setup mount point function*/
int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount_point){
    struct tmpfs_data *td = tmpfs_new_data("directory", "/", NULL, 0);
    struct vnode *vnode = tmpfs_new_vnode(td);

    mount_point->root = vnode;
    vnode->mount = mount_point;

    mount_point->fs = fs;

    printf("finish set up\n");
    printf("root : %s, type : %s\n", td->name, td->type);
    return 1;
}

/*tmpfs file operation*/
int tmpfs_write(struct file* file, const void* buf, size_t len){
    struct vnode *vnode = file->vnode;
    struct tmpfs_data *td = (struct tmpfs_data *)vnode->internal;
    struct tmpfs_filedata *tfd = (struct tmpfs_filedata *)td->data;
    int ret_len = 0;
    
    int start = file->f_pos, end = file->f_pos + len;
    //printf("write data start : %d, end : %d\n", start, end);
    //printf("buf : %s\n", (char *)buf);
    char *data_pos = (char *)tfd->data + start;
    for(int i=start; i<end; i++){
        *data_pos = *((char *)(buf + i - start));
        //printf("-data : %c, buf : %c\n", *data_pos, *((char *)(buf + i - start)));
        data_pos++;
        ret_len++;
    }
    *data_pos = '\0';

    if(ret_len == len) {
        file->f_pos += len;    
        return ret_len;
    }
    else return ERR_SIZE_FAULT;
}

int tmpfs_read(struct file* file, void* buf, size_t len){
    struct vnode *vnode = file->vnode;
    struct tmpfs_data *td = (struct tmpfs_data *)vnode->internal;
    struct tmpfs_filedata *tfd = (struct tmpfs_filedata *)td->data;
    int ret_len = 0;

    int start = file->f_pos, end = file->f_pos + len;
    char *tmp_buf = (char *)simple_malloc(sizeof(char) * 1024);
    for(int i=start; i<end; i++){
        *tmp_buf = *(char *)(tfd->data + i);
        //printf("tmp_buf : %c, data : %c\n", *tmp_buf, *(char *)(tfd->data+i));
        tmp_buf++;
        ret_len++;
    }
    tmp_buf[len] = '\0';
    tmp_buf -= len;
    strcpy(buf, tmp_buf);
    //printf("buf : %s, tmp_buf : %s\n", buf, tmp_buf);

    if(ret_len == len) {
        file->f_pos += len;
        return ret_len;
    }
    else return ERR_SIZE_FAULT;
}

//TODO : flags
int tmpfs_open(struct vnode* file_node, struct file **target){
    (*target)->vnode = file_node;
    (*target)->f_pos = 0;
    (*target)->f_ops = file_node->f_ops;
    return 1;
}

int tmpfs_close(struct file *file){
    return 1;
}

/*tmpfs vnode opearation*/
//TODO : check vnode->mount
int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name){
    //printf("\n\nlook at node : %s\n", ((struct tmpfs_data *)(dir_node->internal))->name);
    if(strcmp(((struct tmpfs_data *)(dir_node->internal))->type, "directory") == 0){
        struct tmpfs_data *td = ((struct tmpfs_data *)(dir_node->internal));

        for(int i=0; i<16; i++){
            struct tmpfs_dirdata *tdd = (struct tmpfs_dirdata *)td->data;
            if(strcmp(tdd->entry[i].name, (char *)component_name) == 0){   //found file
                //printf("tdd[%d] name : %s\n", i, tdd->entry[i].name);
                //printf("component name : %s\n", component_name);
                struct vnode *node = tdd->entry[i].next->node;
                *target = (node == NULL) ? tmpfs_new_vnode(tdd->entry[i].next) : node;
                //printf("node->name : %s\n", ((struct tmpfs_data *)(node->internal))->name );
                //printf("found file\n");
                return SUCCESS_FOUND_FILE;
            }
        }
        //printf("\nfile not fuond\n");
        return ERR_FILE_NOT_FOUND; //file not found
    }
    return ERR_VNODE_NOT_DIR;  //vnode is point to filedata
}

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name){
    //printf("tmpfs_create : %s\n", component_name);
    if(strcmp(((struct tmpfs_data *)(dir_node->internal))->type, "directory") == 0){
        struct tmpfs_data *td = ((struct tmpfs_data *)(dir_node->internal));

        for(int i=0; i<16; i++){
            struct tmpfs_dirdata *tdd = (struct tmpfs_dirdata *)td->data;
            if(strcmp(tdd->entry[i].name, (char *)component_name) == 0){   //found file
                printf("file already exist\n");
                return ERR_FILE_EXIST;
            }
        }
        //find empty entry
        for(int i=0; i<16; i++){
            struct tmpfs_dirdata *tdd = (struct tmpfs_dirdata *)td->data;
            if(tdd->entry[i].next == NULL){   //create file
                tdd->entry[i].next = tmpfs_create_file("file", component_name);
                strcpy(tdd->entry[i].name, component_name);
                //printf("(create) tdd[%d]->name : %s\n", i, tdd->entry[i].name);
                //printf("(create) component name : %s\n", component_name);
                //printf("(create) node name : %s\n", ((struct tmpfs_data*)(tdd->entry->next->node->internal))->name);
                *target = tdd->entry[i].next->node;
                return SUCCESS_CREATE_FILE;
            }
        }
    }
    printf("vnode type is not directory\n");
    return ERR_VNODE_NOT_DIR;
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name){
    //printf("finding : %s\n", component_name);
    if(strcmp(((struct tmpfs_data *)(dir_node->internal))->type, "directory") == 0){
        struct tmpfs_data *td = ((struct tmpfs_data *)(dir_node->internal));

        for(int i=0; i<16; i++){
            struct tmpfs_dirdata *tdd = (struct tmpfs_dirdata *)td->data;
            if(strcmp(tdd->entry[i].name, (char *)component_name) == 0){   //found file
                printf("file already exist\n");
                return ERR_FILE_EXIST;
            }
        }
        //find empty entry
        for(int i=0; i<16; i++){
            struct tmpfs_dirdata *tdd = (struct tmpfs_dirdata *)td->data;
            if(tdd->entry[i].next == NULL){   //create dir
                //printf("tmpfs create dir at entry[%d]\n", i);
                tdd->entry[i].next = tmpfs_create_file("directory", component_name);
                strcpy(tdd->entry[i].name, component_name);
                //printf("create dir name : %s\n", tdd->entry[i].next->name);
                *target = tdd->entry[i].next->node;
                return SUCCESS_CREATE_FILE;
            }
        }
    }
    printf("vnode type is not directory\n");
    return ERR_VNODE_NOT_DIR;
}

/*other tmpfs functions*/
//TODO : size ?? should be size of the dir/file
struct tmpfs_data *tmpfs_new_data(char *type, const char *name, struct vnode *self, int size){
    struct tmpfs_data *td = simple_malloc(sizeof(struct tmpfs_data));
    td->type = type;
    td->name = (char *)name;
    td->node = self;
    td->size = size;
    if(strcmp(type, "directory") == 0) td->data = (void *)tmpfs_new_dirdata();
    else td->data = (void *)tmpfs_new_filedata();
    return td;
}

struct tmpfs_dirdata *tmpfs_new_dirdata(){
    struct tmpfs_dirdata *dirdata = simple_malloc(sizeof(struct tmpfs_dirdata));
    for(int i=0; i<16; i++){
        dirdata->entry[i].next = NULL;
        dirdata->entry[i].name[0] = '\0';
    }

    return dirdata;
}

struct tmpfs_filedata *tmpfs_new_filedata(){
    struct tmpfs_filedata *filedata = simple_malloc(sizeof(struct tmpfs_filedata));
    filedata->data = NULL;
    return filedata;
}

struct vnode *tmpfs_new_vnode(struct tmpfs_data *td){
    struct vnode *new_vnode = (struct vnode *)simple_malloc(sizeof(struct vnode));
    new_vnode->mount = NULL;

    new_vnode->v_ops = (struct vnode_operations *)simple_malloc(sizeof(struct vnode_operations));
    new_vnode->v_ops->create = tmpfs_create;
    new_vnode->v_ops->lookup = tmpfs_lookup;
    new_vnode->v_ops->mkdir = tmpfs_mkdir;

    new_vnode->f_ops = (struct file_operations *)simple_malloc(sizeof(struct file_operations));
    new_vnode->f_ops->read = tmpfs_read;
    new_vnode->f_ops->write = tmpfs_write;
    new_vnode->f_ops->close = tmpfs_close;
    new_vnode->f_ops->open = tmpfs_open;

    //point to each other
    new_vnode->internal = td;
    td->node = new_vnode;

    return new_vnode;
}

struct tmpfs_data *tmpfs_create_file(char *type, const char *name){
    struct vnode *vnode = tmpfs_new_vnode(NULL);
    struct tmpfs_data *td = tmpfs_new_data(type, name, vnode, 0);

    vnode->internal = td;
    td->node = vnode;
    return td;
}

void mount_initramfs(){
    printf("\nmount initramfs\n");
    char dir[1024];
    strcpy(dir, "/initramfs/");
    vfs_mkdir(dir);

    char *addr = (char *)cpio_addr;
    while(!memcmp(addr, "070701", 6) && memcmp(addr+sizeof(cpio_t), "TRAILER!!!", 9)){
        cpio_t *header = (cpio_t *)addr;
        int ns = hex2dec(header->c_namesize, 8);
        int fs = hex2dec(header->c_filesize, 8);

        char filename[1024];
        strclear(filename);
        strncpy(filename, addr+sizeof(cpio_t), ns);
        //printf("filename : %s\n", filename);
        open_initramfs_file(addr, dir, filename, ns, fs);

        //align
        if((fs%4) != 0) fs += 4 - (fs%4);
        if( ((sizeof(cpio_t) + ns) % 4) != 0 ) ns += 4 - ((sizeof(cpio_t)+ns)%4);
        addr += (sizeof(cpio_t) + ns + fs);
    }
}

void open_initramfs_file(char *addr, char *dir, char *filename, int ns, int fs){
    char filepath[1024];
    char buf[248000];

    strclear(filepath);
    strcat(filepath, dir);
    strcat(filepath, filename);
    //printf("filepath : %s\n", filepath);
    
    struct file* fd = vfs_open(filepath, O_CREAT);
    memcpy(buf, addr+sizeof(cpio_t)+ns, fs);
    vfs_write(fd, buf, fs);
    vfs_close(fd);
}