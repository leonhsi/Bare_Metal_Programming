#include "uart.h"
#include "dtb.h"
#include "utils.h"

unsigned long long dtb_addr;

typedef struct{
	unsigned int  magic;
	unsigned int  totalsize;
	unsigned int  off_dt_struct;
	unsigned int  off_dt_strings;
	unsigned int  off_mem_rsvmap;
	unsigned int  version;
	unsigned int  last_comp_version;
	unsigned int  boot_cpuid_phys;
	unsigned int  size_dt_strings;
	unsigned int  size_dt_struct;
}fdt_t;

typedef struct{
	unsigned int tag;
	char name[0];
}fdt_node_header;

typedef struct{
	int tag;
	int len;
	int nameoff;
	char data[0];
}fdt_property;

//convert endian
int big2small(int num){
	int swapped = ((num>>24)&0xff) | // move byte 3 to byte 0
		((num<<8)&0xff0000) | // move byte 1 to byte 2
		((num>>8)&0xff00) | // move byte 2 to byte 1
		((num<<24)&0xff000000); // byte 0 to byte 3

	return swapped;
}

void parse_dtb(void (*get_cpio_addr)(unsigned int)){

	fdt_t* dtb_header = (fdt_t *)dtb_addr;

	unsigned int struct_offset = big2small(dtb_header->off_dt_struct);
	unsigned int string_offset = big2small(dtb_header->off_dt_strings);

	char *addr = (char *)dtb_addr;
	addr += struct_offset;

	while(1){
		unsigned int token = big2small(*(unsigned int *)addr);

		if(token == FDT_BEGIN_NODE){
			fdt_node_header *header = (fdt_node_header *)addr;
			int begin_len = strlen(header->name);

			addr += begin_len + 4;

			if(begin_len % 4 != 0){
				addr += 4 - (begin_len % 4);
			}
		}
		else if(token == FDT_PROP){
			fdt_property *prop = (fdt_property *)addr;
			int prop_len = big2small(prop->len);	

			char *string_addr = (char *)(dtb_addr + string_offset + big2small(prop->nameoff));
			
			if(!strcmp(string_addr, "linux,initrd-start")){
				unsigned int cpio_addr = big2small(*(unsigned int *)(addr + 12));
				(get_cpio_addr)(cpio_addr);

				/*
				int tmp = 0;
				while(tmp < prop_len){
					printf("%c", *(addr+12+tmp));
					tmp++;
				}
				printf("\n");
				*/
			}

			addr += prop_len + 12;

			if(prop_len % 4 != 0){
				addr += 4 - (prop_len % 4);
			}
		}
		else if(token == FDT_END){
			return;
		}
		else{
			addr += 4;
		}
	}
}

void set_dtb_addr(long long addr){
	dtb_addr = addr;
}
