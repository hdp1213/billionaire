BUILDDIR := build

all: .base billionaire-server

.base:
	if ! [ -e $(BUILDDIR) ]; then mkdir $(BUILDDIR) ; mkdir $(BUILDDIR)/lib; fi;
	touch $(BUILDDIR)/.base

vpath %.c src:tests
vpath %.o $(BUILDDIR)
vpath .base $(BUILDDIR)

# Compilers
CC = gcc

# Flags
OPTFLAGS = -O2 -pipe -march=armv6zk -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
DBUG = -g -Wall -Wextra -Wcast-align# -fprofile-arcs -ftest-coverage -pg

CCFLAGS = -fPIC -std=c11 $(OPTFLAGS) $(DBUG)
LDFLAGS = -fPIC -std=c11 $(OPTFLAGS) $(DBUG)

# Includes and libraries
INCLUDES = -Iinclude
LIBS = -levent -lrt -lm -ljson-c -lxxhash
CHECK_LIBS = -ljson-c -lcheck

# Object files to compile
MAIN = billionaire-server.o
SOURCE = billionaire.o book.o card_location.o card_array.o client.o command_error.o game_state.o utils.o

CHECK_BOOK = check_book.o
CHECK_CARD_LOCATION = check_card_location.o
CHECK_UTILS = check_utils.o
MEM_TEST = mem_test.o

# Rules
%.o: %.c .base
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $(addprefix $(BUILDDIR)/, $(notdir $*.o))

billionaire-server: $(MAIN) $(SOURCE)
	$(CC) $(LDFLAGS) -o $@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(LIBS)

check_book: $(CHECK_BOOK) book.o card_location.o command_error.o utils.o
	$(CC) $(LDFLAGS) -o $@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(CHECK_LIBS)

check_card_location: $(CHECK_CARD_LOCATION) card_location.o command_error.o card_array.o utils.o
	$(CC) $(LDFLAGS) -o $@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(CHECK_LIBS)

check_utils: $(CHECK_UTILS) command_error.o utils.o
	$(CC) $(LDFLAGS) -o $@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(CHECK_LIBS)

mem_test: $(MEM_TEST) $(SOURCE)
	$(CC) $(LDFLAGS) -o $@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(LIBS)

check: check_book check_card_location check_utils
	$(addsuffix ;, $(addprefix ./, $^))

clean:
	rm -f billionaire-server
	rm -rf $(BUILDDIR)/*
