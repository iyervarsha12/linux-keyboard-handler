
obj-m += ioctlqueue.o

all:
		make -C /usr/src/linux-2.6.33.2 SUBDIRS=$PWD modules
clean:
		make -C /usr/src/linux-2.6.33.2 SUBDIRS=$PWD clean