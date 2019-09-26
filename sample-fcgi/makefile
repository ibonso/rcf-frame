

#library ubuntu 18.04: /usr/lib/gcc/x86_64-linux-gnu/7/libstdc++.a 
#library ubuntu 16.04: /usr/lib/gcc/x86_64-linux-gnu/5/libstdc++.a

LIBCPP = /usr/lib/gcc/x86_64-linux-gnu/5/libstdc++.a
LIBCPP7= /usr/lib/gcc/x86_64-linux-gnu/7/libstdc++.a

HOST=$(shell hostname)

#beaglebone library diferent path
ifeq ($(HOST), beaglebone)
    LIBCPP = /usr/lib/gcc/arm-linux-gnueabihf/4.9.2/libstdc++.a
endif

ifeq ($(HOST), vps597716)
    LIBCPP = /usr/lib/gcc/x86_64-linux-gnu/7/libstdc++.a
endif




include ../kcgi-0.10.7/Makefile.configure

	
sample-fcgi.o: sample-fcgi.c
	$(CC) -o $@ -c sample-fcgi.c


rcf-frame-fcgi: sample-fcgi.o rcf-proxy.a libkcgi.a 
	$(CC) $(DEBUG) -static -Wall -pthread sample-fcgi.o  libkcgi.a  libsqlite3.a libkcgihtml.a rcf-proxy.a -lz $(LIBCPP)  -lcrypto  -lsqlite3  -ldl -lm -o $@



all: rcf-frame-fcgi

clean:
	rm *.o
	rm rcf-proxy.a
	rm rcf-frame-fcgi
