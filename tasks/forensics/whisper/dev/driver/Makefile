obj-m += whisper.o
KVERSION = 5.4.207
#ccflags-y := -O0

all:
	make -C /home/user/aero-dev/kernel-crackme/linux-5.4.207/ M=$(PWD) modules
clean:
	make -C /home/user/aero-dev/kernel-crackme/linux-5.4.207/ M=$(PWD) clean

