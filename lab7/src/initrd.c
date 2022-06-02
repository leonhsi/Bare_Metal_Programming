#include "uart.h"
#include "utils.h"
#include "initrd.h"

unsigned long cpio_addr;

void parse_cpio_name(){

	printf("\n");

	char *addr = (char *)cpio_addr;

	while(!memcmp(addr, "070701", 6) && memcmp(addr+sizeof(cpio_t), "TRAILER!!!", 9)){
		cpio_t *header = (cpio_t *)addr;
		int ns = hex2dec(header->c_namesize, 8);
		int fs = hex2dec(header->c_filesize, 8);

		//align
		if((fs%4) != 0){
			fs += 4 - (fs%4);
		}
		if( ((sizeof(cpio_t) + ns) % 4) != 0){
			ns += 4 - (sizeof(cpio_t)+ns)%4;
		}

		printf("%s\n", addr+sizeof(cpio_t));
		addr += (sizeof(cpio_t) + ns + fs);
	}
}

void parse_cpio_file(){

	printf("\nFilename: ");

	char c;
	char filename[100];
	int len = 0;

	while((c = uart_getc()) != '\n'){
		uart_send(c);
		filename[len] = c;
		len++;
	}

	char *addr = (char *)cpio_addr;

	while(!memcmp(addr, "070701", 6) && memcmp(addr+sizeof(cpio_t), "TRAILER!!!", 9)){

		cpio_t *header = (cpio_t *)addr;
		int ns = hex2dec(header->c_namesize, 8);
		int fs = hex2dec(header->c_filesize, 8);

		int file_len;
		if((ns-1) > len) file_len = (ns-1);
		else file_len = len;

		//align
		if((fs%4) != 0){
			fs += 4 - (fs%4);
		}
		if(((sizeof(cpio_t) + ns) % 4) != 0){
			ns += 4 - (sizeof(cpio_t)+ns)%4;
		}

		if(!memcmp(addr+sizeof(cpio_t), filename, file_len)){
			printf("\n%s\n", addr+sizeof(cpio_t)+ns);

			return;
		}

		addr += (sizeof(cpio_t) + ns + fs);
	}

	printf("\nFile not found\n");
}

char *get_cpio_file(char *filename){
	char *addr = (char *)cpio_addr;

	int len = strlen(filename);
	
	while(!memcmp(addr, "070701", 6) && memcmp(addr+sizeof(cpio_t), "TRAILER!!!", 9)){

		cpio_t *header = (cpio_t *)addr;
		int ns = hex2dec(header->c_namesize, 8);
		int fs = hex2dec(header->c_filesize, 8);

		int file_len;
		if((ns-1) > len) file_len = (ns-1);
		else file_len = len;

		//align
		if((fs%4) != 0){
			fs += 4 - (fs%4);
		}
		if(((sizeof(cpio_t) + ns) % 4) != 0){
			ns += 4 - (sizeof(cpio_t)+ns)%4;
		}

		if(!memcmp(addr+sizeof(cpio_t), filename, file_len)){
			//printf("\n%s\n", addr+sizeof(cpio_t)+ns);

			return (addr + sizeof(cpio_t) + ns);
		}

		addr += (sizeof(cpio_t) + ns + fs);
	}

	printf("file not found\n");
	return "";
}

int get_cpio_file_len(char *filename){
	char *addr = (char *)cpio_addr;

	int len = strlen(filename);
	
	while(!memcmp(addr, "070701", 6) && memcmp(addr+sizeof(cpio_t), "TRAILER!!!", 9)){

		cpio_t *header = (cpio_t *)addr;
		int ns = hex2dec(header->c_namesize, 8);
		int fs = hex2dec(header->c_filesize, 8);

		int file_len;
		if((ns-1) > len) file_len = (ns-1);
		else file_len = len;

		//align
		if((fs%4) != 0){
			fs += 4 - (fs%4);
		}
		if(((sizeof(cpio_t) + ns) % 4) != 0){
			ns += 4 - (sizeof(cpio_t)+ns)%4;
		}

		if(!memcmp(addr+sizeof(cpio_t), filename, file_len)){
			//printf("\n%s\n", addr+sizeof(cpio_t)+ns);

			return fs;
		}

		addr += (sizeof(cpio_t) + ns + fs);
	}

	printf("file not found\n");
	return 0;
}

void get_initrd_start(char *prop_name, unsigned int addr){
	if(strcmp(prop_name, "linux,initrd-start") == 0){
		//printf("found cpio address : %x\n", addr);
		cpio_addr = addr;
	}

	return;
}
