all:
	gcc tiny_shell.c -o tshell

fork:	tiny_shell.c
	gcc -D FORK tiny_shell.c -o tshell

vfork:	tiny_shell.c
	gcc -D VFORK tiny_shell.c -o tshell

clone:	tiny_shell.c
	gcc -D CLONE tiny_shell.c -o tshell

system: tiny_shell.c
	gcc tiny_shell.c -o tshell

pipe_read:	tiny_shell.c
	gcc -D PIPE_READ tiny_shell.c -o tshell_read

pipe_write:	tiny_shell.c
	gcc -D PIPE_WRITE tiny_shell.c -o tshell_write

clean:
	rm tshell tshell_read tshell_write

