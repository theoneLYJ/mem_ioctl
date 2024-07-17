obj-m += kernel_drv.o
KVERSION = $(shell uname -r)
KERN_DIR =  /lib/modules/$(KVERSION)/build

all:
	make -C $(KERN_DIR) M=$(PWD) modules
	gcc mem_app.c -o mem_app

clean:
	make -C $(KERN_DIR) M=$(PWD) clean
	rm ./mem_app

