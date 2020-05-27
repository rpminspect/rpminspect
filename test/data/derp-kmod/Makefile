# Try to build for the most recent kernel we have development files
# for.  Of all the kernels available with build files, version sort
# the paths and take the most recent one.  Then take the dirname of
# the resulting file to get our kernel build directory.
KDIR = $(shell dirname $(shell ls -1 /lib/modules/*.*.*-*/build/Makefile 2>/dev/null | sort -V | tail -n 1))

obj-m += derp.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
