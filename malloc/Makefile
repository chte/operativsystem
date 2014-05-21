STRATEGY=0

XFLAGS	= -g -Wall -ansi -DSTRATEGY=$(STRATEGY)

CFLAGS	= -g -Wall -DSTRATEGY=$(STRATEGY)

#CC	= gcc -ansi -pedantic -Wall -g -pipe -O -pg
CC	= gcc 


MALLOC=

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

#BIN = tstalgorithms tstextreme tstmalloc tstmemory tstrealloc tstmerge tstincrmalloc
BIN = $(patsubst tst%.c, tst%, $(filter tst%.c, $(SRC)))


# tst%: tst%.o $(MALLOC:.c=.o) $(X)
# 	$(CC) $(CFLAGS) -o $@ tstmerge.o malloc.o $(X)

# tst%: tst%.o $(MALLOC:.c=.o) 
# 	$(CC) $(CFLAGS) -o $@ $^

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


# tstmerge: tstmerge.o malloc.o $(X)
# 	$(CC) $(CFLAGS) -o $@ tstmerge.o malloc.o $(X)

# tstalgorithms: tstalgorithms.o malloc.o $(X)
# 	$(CC) $(CFLAGS) -o $@ tstalgorithms.o malloc.o $(X)

# tstextreme: tstextreme.o malloc.o $(X)
# 	$(CC) $(CFLAGS) -o $@ tstextreme.o malloc.o $(X)

# tstmalloc: tstmalloc.o malloc.o $(X)
# 	$(CC) $(CFLAGS) -o $@ tstmalloc.o malloc.o $(X)

# tstmemory: tstmemory.o malloc.o $(X)
# 	$(CC) $(CFLAGS) -o $@ tstmemory.o malloc.o $(X)

# tstrealloc: tstrealloc.o malloc.o $(X)
# 	$(CC) $(CFLAGS) -o $@ tstrealloc.o malloc.o $(X)

# tstmerge: tstmerge.o malloc.o $(X)
# 	$(CC) $(CFLAGS) -o $@ tstmerge.o malloc.o $(X)


clean:
	\rm -f $(BIN) $(OBJ) core

cleanall: clean
	\rm -f *~