obj-m := char_dev.o
    KDIR ?= /lib/modules/$(shell uname -r)/build
    PWD := $(shell pwd)
all:
	$(MAKE) -C $(KDIR) M=$(PWD)
 
.PHONY:clean
clean:
	rm -f *.mod.c *.mod.o *.ko *.o *.tmp_versions

