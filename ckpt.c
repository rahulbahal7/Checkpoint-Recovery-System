#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ucontext.h>
#include <stdbool.h>

// Header section for the memory sections
struct section{
	long long start_addr;
	long long end_addr;
	bool isContext;
}h1;

// Context variable for the registers
ucontext_t s;
char low[20], high[20];
unsigned long long low_value, high_value;

// Parse the memory maps and get the start, end addresses for
// each of the section.
void readmaps(FILE *fp) {
	char line[1000];
	int i, j, size_struct;
	size_struct = sizeof(h1);
	if (fgets(line, 1000, fp) != NULL) {

		// Parse the Low address
		for (i = 0, j = 0; line[i]!= '-'; i++, j++)
			low[j]= line[i];
		low[j]='\0';

		// Parse the High address
		for (j = 0, i++; line[i] != ' '; i++, j++)
			high[j]= line[i];
		high[j]='\0';

		// Convert addresses to hex
		low_value = strtoll(low, NULL, 16);
		high_value= strtoll(high, NULL, 16);
		h1.start_addr=low_value;
		h1.end_addr=high_value;
		h1.isContext = false;
		return;
	}
}

void checkpoint() {
	FILE *proc_file = fopen("/proc/self/maps", "r");
	char line[1000];
	int l=0;
	while (fgets(line, 1000, proc_file) != NULL) l++;
	int checkpoint_image =  open("myckpt", O_RDWR|O_CREAT,0777);
	rewind(proc_file);
	// Write the memory sections line by line by reading the process memory 
	// maps.
	while(l--) {
		readmaps(proc_file);
		write(checkpoint_image, &h1, sizeof(h1));
		write(checkpoint_image, (char *) low_value, high_value-low_value);
	}
	close(checkpoint_image);
}

// Create the checkpoint and get the register context at the point of 
// receiving the signal
void checkpoint_handler(int signum) {

	//Checkpoint the memory sections
	checkpoint();
	// Get the register context
	int a =getcontext(&s);
	// The following check is to see if we are following the program
	// execution from the above getcontext() statement or returning
	// from setcontext() as it will set the registers back to this state
	if (!(access("context_ckpt", F_OK)!=-1))
	{
		int context_file = open("context_ckpt", O_RDWR|O_CREAT, 0777);
		int bytes_written = write(context_file, &s, sizeof(s));
		close(context_file);
		exit(0);
	}
	return;
}

//Custom signal handler defined as __attribute__((constructor)) so that it 
//executes before the main function

__attribute__((constructor)) void init_signal() {
	signal(SIGUSR2, checkpoint_handler);
}

