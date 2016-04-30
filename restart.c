#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <errno.h>
#include <stdint.h>

typedef unsigned long long ll;
ucontext_t context;
char ckpt_image[1000];

// Structure for the sections
struct section{
	ll start,end;
	bool isContext;
}h1;

// Get the parsed addresses. Also map the memory sections and register context
void getmapdata() {
	int fd= open(ckpt_image, O_RDONLY);
	
	//Map the memory from the start address of each section
	while(true) {
		int val =read(fd, &h1, sizeof(h1));
		if (val ==-1) {
			perror("Error while reading!");
			exit(0);
		}
		
		size_t size=h1.end-h1.start;
		munmap((void *)h1.start, size);
		
		if (h1.isContext == false && h1.end != h1.start){	
			void *start_addr=(void *)h1.start;
			// Fail if memory not mapped at the specified address
			if (mmap(start_addr,size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED, -1, 0) == MAP_FAILED) {
				perror("Error in mmap!\n");
				exit(0);
			}
			// Read data into that section
			if (read(fd, start_addr, size) != size) {
				perror("read failed");
				exit(0);
			}
		}else{	// Restore the registers
			int context_file = open("context_ckpt", O_RDONLY);
			int bytes_read = read(context_file, &context, sizeof(context));
			if(setcontext(&context) == -1)
			{
				printf("error: %s", strerror(errno));
			}
			break;
		}
	}
	close(fd);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("%s\n", "No input specified");
		exit(0);
	}
	//Copy the name of the checkpoint file in data segment(global variable)
	strcpy(ckpt_image, argv[1]);
	
	// Map the memory and register data
	getmapdata();
	return 0;
}
