# KERNEL_BUILD_DIR is a path like /usr/lib/modules/5.6.0/build and is
# passed in by the test_kmod.py script.

# If you want to build this module manually, run:
#
#     make KERNEL_BUILD_DIR=/usr/lib/modules/$(uname -r)/build
#
# This assumes you have the kernel development files installed on your
# system.

obj-m += derp.o

all:
	$(MAKE) -C $(KERNEL_BUILD_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_BUILD_DIR) M=$(PWD) clean
