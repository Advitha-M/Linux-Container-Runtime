obj-m += monitor.o
CFLAGS = -Wall -Wextra -pthread

all:
	gcc $(CFLAGS) engine.c -o engine
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f engine
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
