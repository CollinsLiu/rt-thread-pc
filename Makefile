TOP := ${PWD}

CROSS ?=

CC := $(CROSS)gcc
AR := $(CROSS)ar
LD := $(CROSS)ld
OBJDUMP := $(CROSS)objdump

CFLAGS := -nostdlib -fno-stack-protector

ARCH ?= ia32
CPU ?= ia32
BOARD ?= pc

COMPONENTDIR := $(TOP)/components
FINSHDIR := $(COMPONENTDIR)/finsh
CPUDIR := $(TOP)/libcpu/$(CPU)
BOARDDIR := $(TOP)/bsp/$(BOARD)
KERNELDIR := $(TOP)/src

export CC AR LD TOP COMPONENTDIR CPUDIR BOARDDIR KERNELDIR

LIBS := libbsp.a libcpu.a libkernel.a libfinsh.a

all: $(LIBS)
	$(CC) $(CFLAGS) $(CPUDIR)/start.o $(BOARDDIR)/libbsp.a $(FINSHDIR)/libfinsh.a \
			$(KERNELDIR)/libkernel.a $(CPUDIR)/libcpu.a -o rt-thread.elf -T $(BOARDDIR)/qemu_ram.lds

dummy:
	@echo "building..."

libbsp.a: dummy
	make -C $(BOARDDIR)

libcpu.a: dummy
	make -C $(CPUDIR)

libkernel.a: dummy
	make -C $(KERNELDIR)

libfinsh.a: dummy
	make -C $(FINSHDIR)

clean:
	make -C $(BOARDDIR) clean
	make -C $(CPUDIR) clean
	make -C $(KERNELDIR) clean
	rm rt-thread.elf
dump:
	$(OBJDUMP) -d rt-thread.elf > rt-thread.elf.log

qemu: rt-thread.elf
	qemu -kernel $<
