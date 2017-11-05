BUILDDIR := build

all: .base billionaire-server

.base:
	if ! [ -e $(BUILDDIR) ]; then mkdir $(BUILDDIR) ; mkdir $(BUILDDIR)/lib; fi;
	touch build/.base

vpath %.c src
vpath %.o build
vpath .base build

# Compilers
CC = gcc

# Flags
OPTFLAGS = -O2 -pipe -march=armv6zk -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
DBUG = -g -Wall -Wextra -Wcast-align# -fprofile-arcs -ftest-coverage -pg

CCFLAGS = -fPIC -std=c11 $(OPTFLAGS) $(DBUG)
LDFLAGS = -fPIC -std=c11 $(OPTFLAGS) $(DBUG)

# Includes and libraries
INCLUDES = -Iinclude
LIBS = -levent -lrt -lm -ljson-c

# Object files to compile
MAIN = billionaire-server.o
SOURCE = billionaire.o card.o game_state.o utils.o

CHECK_CARD = tests/check_card.o

# Rules
%.o: %.c .base
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $(addprefix $(BUILDDIR)/, $(notdir $*.o))

billionaire-server: $(MAIN) $(SOURCE)
	$(CC) $(LDFLAGS) -o $@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(LIBS)

check_card: $(CHECK_CARD) $(SOURCE)
	$(CC) $(LDFLAGS) -o $@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(LIBS) -lcheck

check: check_card
	$(addsuffix ;, $(addprefix ./, $^))

clean:
	rm -f billionaire-server
	rm -rf $(BUILDDIR)/*
