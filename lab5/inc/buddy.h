#include "list.h"

//structure for The Array
#define Page_Size           4096
#define Page_Number         65536
#define Frame_Index         17
//#define Page_Number     245760
//#define Frame_Index     19

#define PAGE_MEMORY_BASE    0x10000000
#define PAGE_MEMORY_END     0x20000000

enum USAGE {Val_A, Val_F, Val_X};

typedef struct{
    int index;
    int value;
    enum USAGE usage;
    struct list_head page_list;
}Page;

// typedef struct{
//     Page pages[Page_Number];
// }The_Array;

typedef struct{
    Page pages[Page_Number];
}The_Array;

//structure for free frame linked list
typedef struct{
    int frame_size;
    struct list_head header_list;
}Frame_Header;

typedef struct{
    Frame_Header frame_header[Frame_Index];
}Frame_List;

//global var
The_Array *the_array;
Frame_List *frame_list;

//functions of buddy system
void init_page_array();
void init_frame_list();
void print_list_content(struct list_head *head);
void print_buddy_system();
char *alloc_page(unsigned int size);
void free_pages(char *ptr);



//dynamic allocator
typedef struct{
    int chunk_size;
    int chunk_num; //number of available chunk
    int max_chunk;
    long long chunk_mask; //1 : allocated, 0 : free
    char *base_addr;
    struct list_head par_page_list;
}Par_Page;

typedef struct{
    Par_Page par_page[Page_Number];
}Par_Array;

typedef struct{
    struct list_head par_list_head;
}Par_List;

//global var
Par_Array *par_array;
Par_List *par_list;

//functions
void init_dynamic_memory();
char *allocate_dynamic_memory(int bytes);
void free_dynamic_memory(char *addr);



//startup allocator
void init_startup_array();
void init_startup_list();
void startup_allocator();
void get_initrd_end(char *prop_name, unsigned int addr);



