ECHO OFF

windres resource.rc -o resource.o

gcc -c main.c -o main.o

gcc main.o resource.o -lgdi32 -o CapsDetect.exe -mwindows 

del main.o resource.o
