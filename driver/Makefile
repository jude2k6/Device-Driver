obj-m += mouse.o
mouse-objs := mouse_driver.o mouse_char.o mouse_usb.o mouse_features.o  mouse_proc.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
