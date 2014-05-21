STRATEGY=0

XFLAGS	= -g -Wall -O4 -ansi -DSTRATEGY=$(STRATEGY)

CFLAGS	= -g -Wall -O4 -DSTRATEGY=$(STRATEGY)

# CC	= gcc -ansi -pedantic -Wall -g -pipe -O -pg
CC	= gcc 


MALLOC=malloc.c

SRC	= $(MALLOC) \
	  tstalgorithms.c  \
	  tstextreme.c \
	  tstmalloc.c \
	  tstmemory.c \
	  tstrealloc.c \
	  tstmerge.c \
	  tstincrmalloc.c

OBJ	= tstalgorithms.o \
	  tstextreme.o \
	  tstmalloc.o \
	  tstmemory.o \
	  tstrealloc.o \
	  tstmerge.o \
	  tstincrmalloc.o

BIN = $(patsubst tst%.c, tst%, $(filter tst%.c, $(SRC)))


.PHONY: all clean

all: $(BIN)
	 exec ./RUN_TESTS

tst%: tst%.o $(MALLOC:.c=.o)
	$(CC) $(XFLAGS) -o $@ $^

# These programs can not be compiled as ANSI-standard C.
tst%.o: tst%.c
	$(CC) -c $(CFLAGS) $< -o $@

# But the rest should be ANSI-standard C.
%.o: %.c
	$(CC) -c $(XFLAGS) $< -o $@

clean:
	\rm -f $(BIN) $(OBJ) core

cleanall: clean
	\rm -f *~