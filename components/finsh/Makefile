SRCS := $(wildcard *.c)

OBJS := $(addsuffix .o, $(basename $(SRCS)))

CFLAGS := -mtune=i386 -fno-stack-protector -I$(TOP)/include -I. -I$(BOARDDIR) -I$(CPUDIR)/include

.c.o:
	$(CC) -c $(CFLAGS) $(SRCS)

libfinsh.a: $(OBJS)
	$(AR) rc $@ $(OBJS)

clean:
	-rm *.o *.a

