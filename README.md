# mini-DMTCP

mini-DMTCP is a Single thread Checkpoint and Recovery System. This means that
using mini-DMTCP a program can be resumed/restored back from the state when it
was terminated. 

The code is written using low-level C.

LOCIC/ALGORITHM behind mini-DMTCP:

Checkpoint:
   1. Read the memory maps for the process being run.
   2. Capture all the memory locations and data in those pages in a checkpoint
      file.
   3. Capture the context of the CPU registers, Stack Pointer and Program Counter
      in another file.

Restart:
   NOTE: Its a good practice(although I did'nt follow it as it's not necessary)
   to map the stack from the original program back in the memory and move the
   stack pointer there before proceeding. This also requires copying the name of
   the input file to the data segment which can easily be done by moving it to a
   global variale.

   1. Read the checkpoint file and map back the memory ranges with the original
      data.
   2. Then restore the context of the registers, Stack Pointer and Program
      Counter which can be restored back to the original program's state using
      setcontext().
   3. Please note that `setcontext()` would try to jump back to the point where
      you set the register state using `getcontext()` (read about it and you'll
      know what I'm talking about here :) !). Hence, you should distinguish
      between these two states and prevent the overwriting of the checkpoint
      image files.

DEMO:
     For demo run the command:
     `make check`  
     
     This will run the test program (an infinite loop) for 3 seconds, kill it,
     restore it back into the memory and restart it back from the point where it
     was killed.

