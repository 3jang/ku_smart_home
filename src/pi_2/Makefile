KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := pi_2.o

default :
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
#	arm-linux-gnueabi-gcc -o dht11_app dht11_app.c -l paho-mqtt3c
#	arm-linux-gnueabi-gcc -o ku_pir_app2 ku_pir_app2.c

copy:
	scp mknod.sh pi_2.ko pi_2_lib.c pi_2_app.c pi@10.10.10.13:~/

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
