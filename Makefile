ifeq ($(BUILD),debug)
# "Debug" build - no optimization, and debugging symbols
CFLAGS += -O0 -g
else
# "Release" build - optimization, and no debug symbols
CFLAGS += -O3 -s -DNDEBUG
endif

CFLAGS += -Werror -Wall -std=c11 -pedantic

all: poker

debug:
	make "BUILD=debug"

poker:
	cc $(CFLAGS) -o poker poker.c
	./poker
