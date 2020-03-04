CC = gcc
CXX = g++

CFLAGS = -Wno-psabi -Wall -std=c++11 -fPIC 

ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -O0 -ggdb
else
    CFLAGS += -DNDEBUG -O3
endif

ENGAGE_VER ?= 1.121.8880
INCLUDES =  -I. -I../../../api/c/include
LFLAGS = -L../../../bin/$(ENGAGE_VER)/darwin_x64
LIBS = -lengage-shared -lpthread -lstdc++

SRCS = Mingage.cpp WorkQueue.cpp
OBJS = $(SRCS:.cpp=.o)

MAIN = mingage

.PHONY: depend clean

all:    $(MAIN)
	@echo Building $(MAIN) with Engage version $(ENGAGE_VER)

$(MAIN): $(OBJS) 
	$(CXX) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.cpp.o:
	$(CXX) -c $(CFLAGS) $(INCLUDES) $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
