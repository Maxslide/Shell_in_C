#to run the command first we have to run this makefile useing make command

dafault: main

main: Shell.c
	gcc Shell.c -o shell -lreadline
