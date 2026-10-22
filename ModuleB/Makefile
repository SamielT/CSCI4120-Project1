# Compiler and flags
CC = gcc
CFLAGS = -Wall -O2

# Targets
all: parent child

parent: Project1_Module_B.c Project1_Module_B.h
	$(CC) $(CFLAGS) -o parent Project1_Module_B.c

child: child_proc.c Project1_Module_B.h
	$(CC) $(CFLAGS) -o child child_proc.c

# Clean build artifacts
clean:
	rm -f parent child
