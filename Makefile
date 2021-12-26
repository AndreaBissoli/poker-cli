CC = gcc
CFLAGS = -Werror -Wall -std=c11 -pedantic

ifeq ($(BUILD),debug)
# "Debug" build - no optimization, and debugging symbols
CFLAGS += -O0 -g
else
# "Release" build - optimization, and no debug symbols
CFLAGS += -O3 -s -DNDEBUG
endif

all: poker

debug:
	make "BUILD=debug"

poker:
	$(CC) $(CFLAGS) -o poker *.c
	./poker -p 3
