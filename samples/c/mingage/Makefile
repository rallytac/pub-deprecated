CC := gcc
CXX := g++

CFLAGS := -Wall -std=c++11 -fPIC 

#CFLGAS += -Wno-psabi

ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -O0 -ggdb
else
    CFLAGS += -DNDEBUG -O3
endif

UNAME_S := $(shell uname -s | tr A-Z a-z)
ifeq ($(UNAME_S), darwin)
	LIBSRC = ../../../bin/latest/darwin_x64
endif

INCLUDES =  -I. -I../../../api/c/include
LFLAGS = -L.
LIBS = -lengage-shared -lpthread -lstdc++

SRCS = Mingage.cpp WorkQueue.cpp
OBJS = $(SRCS:.cpp=.o)

$(info //)
$(info //)
$(info // NOTE: libengage-shared will be copied locally from $(LIBSRC))
$(info //)
$(info //       Be sure to 'export LD_LIBRARY_PATH=./')
$(info //)
$(info //)

MAIN = mingage

.PHONY: depend clean

all:    $(MAIN)

$(MAIN): $(OBJS) 
	@cp $(LIBSRC)/libengage-shared.* .
	$(CXX) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.cpp.o:
	$(CXX) -c $(CFLAGS) $(INCLUDES) $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN) libengage-shared.*

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
