
CC		= g++
GC      =gcc
# -Wstrict-prototypes
CFLAGS		=  -g -W -Wall -Wextra -std=c++11   -Wwrite-strings -Wno-unused-parameter
INCLUDE     = -I /usr/src/linux-headers-4.15.0-30-generic/include/config/
INCLUDEBLUE = -I /usr/src/linux-headers-4.15.0-30-generic/include/config/  -I /usr/include/node/
CPPFLAGS	= 
LDADD		= 
LDFLAGS		= 
STATIC		= 
PREFIX		= /usr/local
BINDIR		= /usr/local/bin
SHAREDIR	= /usr/local/share
SBINDIR		= /usr/local/sbin
INCLUDEDIR	= /usr/local/include
LIBDIR		= /usr/local/lib
MANDIR		= /usr/local/man
INSTALL		= install
INSTALL_PROGRAM	= install -m 0555
INSTALL_LIB	= install -m 0444
INSTALL_MAN	= install -m 0444
INSTALL_DATA	= install -m 0444

GTEST_DIR	= googletest

#beaglebone needs extra include
ifeq ($(hostame), beaglebone)
    INCLUDE = INCLUDEBLUE
endif


# Comment if you don't need statically linked.
# This is only for the sample program!
STATIC 		 = -static

DEBUG		 = -ggdb 


#EXEFILE      = rcf-proxy-test \
#               rcf-serv-r
		      

OBJECTS 	 = adm.o \
			   adm-base.o \
			   adm-session.o \
			   adm-user.o \
			   rcf-proxy.o \
			   rcf-op.o \
			   rcf-op-ping.o \
			   rcf-op-login.o \
			   rcf-op-logout.o \
			   rcf-op-lsdir.o \
			   rcf-op-command.o \
			   rcf-op-service.o \
			   rcf-op-blue.o \
			   rcf-op-sessionuser.o \
			   rcf-op-user.o \
			   util/base64.o \
			   util/jsonutil.o \
#			   rcfBlueProxy.o

			   
				
OBJTEST 	 = rcf-op-login-test.o \
			   rcf-proxy-test.o
  		       

SRCS	 = rcf-fcgi.cpp  \
		   adm-session.cpp	\
		   adm-user.cpp	\
		   rcf-op-login.cpp \
		    
		   
UTILSRCS = util/base64.cpp	util/jsonutil.cpp util/rcf-md5.h	   
UTILOBJ  = base64.o jsonutil.o
		   
VPATH = util





gtestbuild: ${GTEST_DIR}/src/gtest-all.cc
	g++  -std=c++11 -isystem googletest/include -I${GTEST_DIR} \
		-pthread -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o




%.o : %.cpp %.hpp rcf-conf.h
	$(CC)  $(CFLAGS) -c  -pthread -o $@ -c $< $(INCLUDE)




.PHONY: clean	
clean: 
	rm -f $(OBJECTS) 
	rm -f $(OBJTEST) 
	rm -f $(EXEFILE)
	rm -f *.o
	rm -f *.gch
	rm -f *.db
	rm -f rcf-proxy.a
	rm -f rcf-serv-r
	


.PHONY: all

all: rcf-proxy.a  rcf-serv-r

copylib:
	cp   rcf-proxy.a  ../sample-fcgi/rcf-proxy.a


rcf-proxy.o: rcf-proxy.c rcf-proxy.h rcf-conf.h
	g++ $(DEBUG) -Wall -pthread -c -o $@ rcf-proxy.c  -std=c++11 -lstdc++

rcf-proxy.a: $(OBJECTS)
	$(AR) rs $@ $(OBJECTS)
	
rcf-proxy-test.o: rcf-proxy-test.c	 
	g++ $(DEBUG) -Wall -pthread -c rcf-proxy-test.c -std=c++11

rcf-serv-r: rcf-serv-r.c rcf-conf.h
	$(GC) $(DEBUG) rcf-serv-r.c -Wall -pthread -o rcf-serv-r

rcf-serv-b: rcf-serv-b.c rcf-conf.h
	$(GC) $(DEBUG) rcf-serv-b.c -Wall -pthread -o rcf-serv-r


#$@ test in this case
test: adm-user-test adm-session-test rcf-op-login-test rcf-proxy-test
	


