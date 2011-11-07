SRCS := $(wildcard *.c)

OBJS := $(addsuffix .o, $(basename $(SRCS)))

CFLAGS := -fno-stack-protector -I$(TOP)/include -I. -I$(BOARDDIR) -I$(CPUDIR)/include $(CFLAGS_COMMON)

.c.o:
	$(CC) -c $(CFLAGS) $(SRCS)

libfinsh.a: $(OBJS)
	$(AR) rc $@ $(OBJS)

clean:
	-rm *.o *.a

