# Joseph Petitti and Justin Cheng

obj-m := mymodule.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc -Wall test.c -o test

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f test
