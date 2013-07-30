#Makefile for the FMMAVX project

CC=icpc
CFLAGS= -lpthread -rdynamic -Wl,--enable-new-dtags -O0
INCMIC=-mmic /opt/intel/mic/coi/device-linux-release/lib/libcoi_device.so -I/opt/intel/mic/coi/include 
INC=/opt/intel/mic/coi/host-linux-release/lib/libcoi_host.so -I/opt/intel/mic/coi/include 
SRC=./src

debug: CC+= -g
debug: all

all: 
	${CC} ${CFLAGS} ${INCMIC} offload_on_mic.cpp -o offload_on_mic
	${CC} ${CFLAGS} ${INC} upanddown.c -o upanddown

clean:
	rm -f upanddown offload_on_mic


