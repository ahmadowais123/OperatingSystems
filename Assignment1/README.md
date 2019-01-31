# Assignment1 - Tiny Shell

Author: Owais Khan
McGill ID: 260617913

The zip folder includes the following files:
1. tiny_shell.c
2. myheader.h
3. makefile
4. Readme


Instructions on how to run system(), fork(), vfork() and clone() versions:
* For system(): make system
* For fork(): make fork
* For vfork(): make vfork
* For clone(): make clone
* Each of the above commands create an executable called tshell that can be run with the following command: ./tshell


Instructions on how to run the piping implementation using FIFO.

NOTE: Please ensure that the created fifo has sufficient permissions.

1. make pipe_write                   (Creates an executable named tshell_write)
2. make pipe_read                    (Creates an executable named tshell_read)
3. ./tshell_write <path_to_fifo>     (Run the writing end of the pipe in one shell)
4. ./tshell_read <path_to_fifo>      (Run the reading end of the pipe in a new shell)
