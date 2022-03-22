#include "list.h"

void init_page_array();
void init_frame_list();
void print_list_content(struct list_head *head);
void print_buddy_system();
char *alloc_page(unsigned int size);
void free_pages(char *ptr);
void init_dynamic_memory();
char *allocate_dynamic_memory(int bytes);
void free_dynamic_memory(char *addr);