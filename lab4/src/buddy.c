#include "uart.h"
#include "utils.h"
#include "buddy.h"

#define PAGE_MEMORY_BASE    0x10000000
#define PAGE_MEMORY_END     0x20000000
#define Page_Size           4096
#define Page_Number         65536

enum USAGE {Val_A, Val_F, Val_X};

//structure for The Array
typedef struct{
    int index;
    int value;
    enum USAGE usage;
    struct list_head page_list;
}Page;

typedef struct{
    Page pages[Page_Number];
}The_Array;

//structure for free frame linked list
typedef struct{
    int frame_size;
    struct list_head header_list;
}Frame_Header;

typedef struct{
    Frame_Header frame_header[17];
}Frame_List;

//global var
The_Array *the_array;
Frame_List *frame_list;

void init_page_array(){
    the_array = (The_Array *)simple_malloc(sizeof(The_Array));

    for(int i=0; i<Page_Number; i++){
        the_array->pages[i].index = i;
        the_array->pages[i].value = (i==0) ? 16 : -1;
        the_array->pages[i].usage = (i==0) ? Val_A : Val_F;
        INIT_LIST_HEAD(&the_array->pages[i].page_list);
    }
}

void init_frame_list(){
    frame_list = (Frame_List *)simple_malloc(sizeof(Frame_List));

    for(int i=0; i<=16; i++){
        frame_list->frame_header[i].frame_size = i;
        INIT_LIST_HEAD(&frame_list->frame_header[i].header_list);
    }

    //link init free frame node to list
    list_add(&the_array->pages[0].page_list, &frame_list->frame_header[16].header_list);
}

void print_list_content(struct list_head *head){

	struct list_head *pos;
	int num = 0;

	list_for_each(pos, head){
		Page *node = container_of(pos, Page, page_list);
		printf(" -> [idx(%d), val(%d)] ", node->index, node->value);
		num++;
	}

    if(num == 0){
        printf(" empty list");
    }
    printf("\n");
}

void print_buddy_system(){
    for(int i=0; i<17; i++){
        printf("[%d] ", i);
        print_list_content(&(frame_list->frame_header[i].header_list));
    }
}

char *alloc_page(unsigned int size){
    printf("\nallocating [%d] pages\n\n", size);

    int target_frame_index = 0;
    while(size > 1){
        size >>= 1;
        target_frame_index++;
    }

    //printf("start search free frame at list index [%d]\n\n", target_frame_index);

    int empty_frame_index = target_frame_index;
    while(list_empty(&frame_list->frame_header[empty_frame_index].header_list)){
        //printf("frame list index [%d] have no available pages\n\n", empty_frame_index);
        empty_frame_index++;
    }

    if(empty_frame_index <= 16){
        printf("found available frame at frame list index [%d]\n\n", empty_frame_index);
    }
    else{
        printf("can't not allocate pages, page number out of range\n\n");
        return "";
    }

    printf("allocating pages.....\n");
    //printf("=============================================\n");

    int idx = -1;
    int val = -1;
    while(empty_frame_index != target_frame_index){

        //retrive first node on the frame list 
        Page *node = container_of(frame_list->frame_header[empty_frame_index].header_list.next, \
                                Page, \
                                page_list);
        idx = node->index;
        val = node->value;

        //printf("frame list [%d] has node with index [%d] and value [%d]\n\n", empty_frame_index, idx, val);

        //print old info
        //printf("before updating buddy system : \n");
        //print_buddy_system();

        //spliting the array into new buddy
        //printf("\nspliting The Array index [%d] with value [%d] : \n", idx, val);
        if(the_array->pages[idx].usage == Val_X){
            printf("\n\tcan not allocate pages at index %d with value %d\n", idx, the_array->pages[idx].usage);
            return "";
        }
        else{
            the_array->pages[idx].value = val - 1;
            //printf("\t1st : index [%d] with value [%d]\n", idx, val-1);
        }

        int new_buddy_index = idx + pow(2, val-1);
        if(the_array->pages[new_buddy_index].usage == Val_X){
            printf("\n\tcan not allocate pages at index %d with value %d\n", new_buddy_index, the_array->pages[new_buddy_index].value);
        }
        else{
            the_array->pages[new_buddy_index].value = val - 1;
            //printf("\t2nd : index [%d] with value [%d]\n\n", new_buddy_index, val-1);
        }

        //update frame list
        list_del(frame_list->frame_header[empty_frame_index].header_list.next);

        list_add_tail(&(the_array->pages[idx].page_list), &(frame_list->frame_header[empty_frame_index-1].header_list));
        list_add_tail(&(the_array->pages[new_buddy_index].page_list), &(frame_list->frame_header[empty_frame_index-1].header_list));

        //print new info
        //printf("\nupdated buddy system : \n");
        //print_buddy_system();

        empty_frame_index--;
        
        //printf("=============================================\n");
    }

    //delete requested page from frame list
    Page *target_page = container_of(frame_list->frame_header[empty_frame_index].header_list.next, \
                                    Page, \
                                    page_list);
    list_del(&target_page->page_list);

    //mark used page on The Array to not allocable
    int target_index = target_page->index;
    int target_value = target_page->value;
    for(int i=0; i<pow(2,target_value); i++){
        the_array->pages[target_index + i].usage = Val_X;
    }

    //print final info
    printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("final buddy system : \n");
    print_buddy_system();
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    printf("\nallocated page : \n");
    printf("index [%d] with value [%d]\n\n", target_index, target_value);
    
    unsigned long long offset = target_index * Page_Size;
    printf("offset : %x\n\n", offset);
    char *page_ptr = ((char *)PAGE_MEMORY_BASE + offset);
    printf("page allocated at %x, size %d\n", page_ptr, pow(2, target_value));

    return page_ptr;
}

void free_pages(char *ptr){

    //retirrve the pointed address's index and order
    int idx = (int)((double)(hex2dec(ptr,8) - PAGE_MEMORY_BASE) / Page_Size);
    printf("\nreturned page index in The Array : %d\n", idx);

    int order = -1;
    if(the_array->pages[idx].usage == Val_X){
        order = the_array->pages[idx].value;
        printf("\norder of the returned page : %d\n", order);
    }
    else{
        printf("\npage has not been allocated\n");
        return;
    }

    //add returned page to frame list
    list_add_tail(&(the_array->pages[idx].page_list), &(frame_list->frame_header[order].header_list));

    //mark usage on The Array to allocable
    for(int i=0; i<pow(2, order); i++){
        the_array->pages[idx+i].usage = (i==0) ? Val_A : Val_F;
    }

    printf("\nbuddy system after page returned : \n");
    print_buddy_system();

    printf("=============================================\n");

    int cnt = 1;
    while(1){

        //find buddies
        int buddy_idx = idx ^ pow(2,order);
        printf("\nindex : %d\n", idx);
        printf("\nbuddy index : %d\n", buddy_idx);
        printf("\norder of this index : %d\n", order);

        int can_merge = 0;
        struct list_head *pos;
        list_for_each(pos, &frame_list->frame_header[order].header_list){
		    Page *node = container_of(pos, Page, page_list);
		    if(node->index == buddy_idx){
                can_merge = 1;                
                break;
            }
	    }

        if(can_merge){
            //delete both node on the frame list
            list_del(&(the_array->pages[idx].page_list));
            list_del(&(the_array->pages[buddy_idx].page_list));

            //update The Array
            int base_idx = (idx < buddy_idx) ? idx : buddy_idx;
            for(int i=0; i<pow(2, order+1); i++){
                the_array->pages[base_idx+i].usage = (i == 0) ? Val_A : Val_F;
                the_array->pages[base_idx+i].value = (i == 0) ? (order+1) : -1;
            }

            //merge to a new node on the frame list
            list_add_tail(&(the_array->pages[base_idx].page_list), &(frame_list->frame_header[order+1].header_list)); 

            printf("\nbuddy system after %dth merged : \n", cnt);
            print_buddy_system();
        }
        else{
            printf("=============================================\n");
            break;
        }

        order++;
        cnt++;

        printf("=============================================\n");
    }

    printf("\npage reutrned finish\n"); 
    printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("final buddy system : \n");
    print_buddy_system();
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");  
}

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

void init_dynamic_memory(){

    par_array = (Par_Array *)simple_malloc(sizeof(Par_Array));
    for(int i=0; i<Page_Number; i++){
        par_array->par_page[i].chunk_size = -1;
        par_array->par_page[i].chunk_num = 0;
        par_array->par_page[i].max_chunk = 0;
        par_array->par_page[i].chunk_mask = 0;
        par_array->par_page[i].base_addr = (char *)PAGE_MEMORY_END;
        INIT_LIST_HEAD(&(par_array->par_page[i].par_page_list));
    }

    par_list = (Par_List *)simple_malloc(sizeof(Par_List));
    INIT_LIST_HEAD(&(par_list->par_list_head));
}

void print_par_list(){
    printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("contents of partitioned page list : \n");
    printf("head ");
    int num = 0;
    struct list_head *pos;

    list_for_each(pos, &(par_list->par_list_head)){
        Par_Page *node = container_of(pos, Par_Page, par_page_list);
        if(num == 0){
            printf("-> ");
        }
        else{
            printf("\n     -> ");
        }
        printf("[chunk_size(%d), chunk_num(%d), chunk_mask(%d), base_addr(%x)] ", node->chunk_size, node->chunk_num, node->chunk_mask, node->base_addr);
        num++;
    }
    if(num == 0){
        printf("empty list");
    }

    printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

char *allocate_dynamic_memory(int bytes){

    printf("\nrequired bytes : [%d] \n", bytes);

    //page pointer to be returned
    char *ret_page_ptr; 
    
    //find if partitioned page list has same chunk size
    int allocated_pages = 0;

    struct list_head *pos;
    list_for_each(pos, &(par_list->par_list_head)){

        Par_Page *node = container_of(pos, Par_Page, par_page_list);

        //find page, allocate one chunk and return
        //if chunk mask are all 1, need to find another page with same chunk size even if there are free chunks :(
        if( (node->chunk_size == bytes) && (node->chunk_num > 0) && (allOne(node->chunk_mask) == 0) ){     
            //update par page node
            node->chunk_num--;

            int chunk_mask = node->chunk_mask;
            int chunk_idx = 0;
            while(1){
                int cur_idx = chunk_mask & (1);
                //find free chunk
                if(cur_idx == 0){
                    break;
                }
                chunk_mask >>= 1;
                chunk_idx++;
            }
            node->chunk_mask |= (1 << chunk_idx);

            //calculate return address
            ret_page_ptr = node->base_addr;
            int offset = (chunk_idx == 0) ? 0 : chunk_idx;
            ret_page_ptr += offset * node->chunk_size;

            print_par_list();
            
            return ret_page_ptr;
        }

        allocated_pages++;
    }
    
    //can not find page, allocate new partitioned page and return its base address
    //allocate new partitioned page
    ret_page_ptr = alloc_page(1);
    int chunk_size = bytes;
    int max_chunk = Page_Size / chunk_size;

    //update par array
    par_array->par_page[allocated_pages].chunk_size = chunk_size;
    par_array->par_page[allocated_pages].chunk_num = max_chunk - 1; //allocate one chunk
    par_array->par_page[allocated_pages].max_chunk = max_chunk;
    par_array->par_page[allocated_pages].chunk_mask |= 1;           //idx 0 chunk is allocated
    par_array->par_page[allocated_pages].base_addr = ret_page_ptr; 

    //update par list
    list_add_tail(&(par_array->par_page[allocated_pages].par_page_list), &(par_list->par_list_head));
    print_par_list();

    return ret_page_ptr;
}

void free_dynamic_memory(char *addr){

    long long addr_hex = (long long )atoh(addr);
    long long addr_prefix = addr_hex & 0xFFFFF000;
    char *addr_ptr = (char *)(addr_hex);

    struct list_head *pos;

    list_for_each(pos, &(par_list->par_list_head) ){
        Par_Page *node = container_of(pos, Par_Page, par_page_list);

        //find the chunk to be freed belongs to which page by address prefix        
        if( (long long)node->base_addr == addr_prefix ){
            //caculate which chunk index being returned
            int idx = (int)((addr_ptr - node->base_addr) / node->chunk_size);
            printf("\nto be freed chunk index : %d\n", idx);

            //check if chunk index has been allocated
            if(node->chunk_mask & (1<<idx)){ 
                //clear chunk mask
                node->chunk_mask &= ~(1<<idx);
                node->chunk_num++;

                print_par_list();
            }
            else{
                printf("\nthis chunk has not been allocated\n");
            }
        
            return;
        }
    }

    printf("\naddress has no dynamic memory page allocated\n");
    return;
}




