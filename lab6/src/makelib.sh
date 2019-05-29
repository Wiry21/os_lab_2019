rm -f multmod.o libmultmod.a
gcc -o multmod.o -c multmod.c
ar rcs libmultmod.a multmod.o
rm -f multmod.o