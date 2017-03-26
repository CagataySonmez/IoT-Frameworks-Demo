ARCH := $(shell getconf LONG_BIT)

IDIR=-I../sdk/x86_$(ARCH)/include -I../sdk/x86_$(ARCH)/include/resource -I../sdk/x86_$(ARCH)/include/service -I../sdk/x86_$(ARCH)/include/c_common
LDIR=-L../sdk/x86_$(ARCH)/lib
CC=g++
CFLAGS=-Wl,-rpath,../../sdk/x86_$(ARCH)/lib -Wall -std=c++0x $(IDIR) $(LDIR) -loc -loctbstack -loc_logger -lconnectivity_abstraction -lpthread
SERV_OBJ = simpleserver.o
CLIENT_OBJ = simpleclient.o

%.o: %.cpp
	$(CC) -c -o ../bin/x86_$(ARCH)/$@ $^ $(CFLAGS)

#Build the simpleserver executable
all:simpleserver simpleclient
simpleserver:$(SERV_OBJ)
	$(CC) -o ../bin/x86_$(ARCH)/$@ ../bin/x86_$(ARCH)/$^ $(CFLAGS)

#Build the simpleclient executable
simpleclient:$(CLIENT_OBJ)
	$(CC) -o ../bin/x86_$(ARCH)/$@ ../bin/x86_$(ARCH)/$^ $(CFLAGS) 
