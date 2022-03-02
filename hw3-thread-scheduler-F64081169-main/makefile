
GIT_HOOKS := .git/hooks/applied
CC := gcc
CFLAGS += -std=gnu99 -g -Wall

all: $(GIT_HOOKS) 

$(GIT_HOOKS):
	@.githooks/install-git-hooks
	@echo

simulator:simulator.o os2021_thread_api.o function_libary.o
	$(CC) $(CFLAGS) -o simulator simulator.o os2021_thread_api.o function_libary.o -ljson-c

simulator.o:simulator.c os2021_thread_api.h
	$(CC) $(CFLAGS) -c simulator.c

os2021_thread_api.o:os2021_thread_api.c os2021_thread_api.h function_libary.h
	$(CC) $(CFLAGS) -c os2021_thread_api.c

function_libary.o: function_libary.c function_libary.h
	$(CC) $(CFLAGS) -c function_libary.c

.PHONY: clean
clean:
	rm *.o
