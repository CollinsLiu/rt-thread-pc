SRCS := clock.c device.c idle.c ipc.c irq.c kservice.c mem.c mempool.c object.c scheduler.c slab.c thread.c timer.c

OBJS := $(addsuffix .o, $(basename $(SRCS)))

CFLAGS := -mtune=i386 -fno-stack-protector -I$(TOP)/include -I. -I$(BOARDDIR) -I$(CPUDIR)/include

.c.o:
	$(CC) -c $(CFLAGS) $(SRCS)

libkernel.a: $(OBJS)
	$(AR) rc $@ $(OBJS)

clean:
	-rm *.o *.a

