CC = gcc
CFLAGS = -g -O2 -Wall -W
AR = ar
RM = rm -f

SOURCES = cluster.c
OBJS = cluster.o

all: lib.a

lib.a: $(OBJS)
	$(AR) crs lib.a $(OBJS)

dep: $(SOURCES)
	$(CC) $(CFLAGS) -MM $(SOURCES) >> Makefile

clean:
	$(RM) $(OBJS) lib.a

distclean:
	$(RM) Makefile

# automatically generated dependencies follow
