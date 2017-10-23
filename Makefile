BUILDDIR := build

all: .base billionaire-server

.base:
	if ! [ -e $(BUILDDIR) ]; then mkdir $(BUILDDIR) ; mkdir $(BUILDDIR)/lib; fi;
	touch build/.base

vpath %.c src
vpath %.o build
vpath .base build


CC = gcc

OPTFLAGS = -O2 -pipe -march=armv6zk -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
DBUG = -g -Wall -Wextra -Wcast-align

CCFLAGS = -fPIC $(OPTFLAGS) $(DBUG)
LDFLAGS = -fPIC $(DBUG) $(OPTFLAG)

INCLUDES = -Iinclude
LIBS = -levent -lrt -lm -ljson-c

SOURCE = billionaire.o billionaire-server.o
# MAIN = main.o

%.o: %.c .base
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $(BUILDDIR)/$*.o

billionaire-server: $(SOURCE)
	$(CC) $(LDFLAGS) -o $@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(LIBS)

clean:
	rm -f billionaire-server *~
	rm -rf $(BUILDDIR)/*
