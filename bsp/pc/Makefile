SRCS := startup.c application.c board.c console.c keyboard.c serial.c

ASMSRCS := 

OBJS := $(addsuffix .o, $(basename $(SRCS)))

OBJS += $(addsuffix .o, $(basename $(ASMSRCS)))

CFLAGS := -I$(TOP)/include -I. -I$(BOARDDIR) -I$(CPUDIR)/include $(CFLAGS_COMMON)

.S.o:
	$(CC) -c $(CFLAGS) $(ASMSRCS)
.c.o:
	$(CC) -c $(CFLAGS) $(SRCS)

libbsp.a: $(OBJS)
	$(AR) rc $@ $(OBJS)

clean:
	-rm *.o *.a

