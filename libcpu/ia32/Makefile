SRCS := backtrace.c interrupt.c showmem.c stack.c string.c trap.c __udivsi3.c __umodsi3.c

ASMSRCS := start.S context.S hdisr.S trapisr.S

OBJS := $(addsuffix .o, $(basename $(SRCS)))

OBJS += $(addsuffix .o, $(basename $(ASMSRCS)))

CFLAGS := -fno-stack-protector -I$(TOP)/include -I. -I$(BOARDDIR) -I$(CPUDIR)/include $(CFLAGS_COMMON)

.S.o:
	$(CC) -c $(CFLAGS) $(ASMSRCS)
.c.o:
	$(CC) -c $(CFLAGS) $(SRCS)

libcpu.a: $(OBJS)
	$(AR) rc $@ $(OBJS)

clean:
	-rm *.o *.a
