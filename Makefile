SRCDIR := src
BINDIR := bin
BUILDDIR := build
SRCEXT := c

all: .base billionaire-server

.base:
	if ! [ -e $(BINDIR) ]; then mkdir $(BINDIR); fi;
	if ! [ -e $(BUILDDIR) ]; then mkdir $(BUILDDIR); fi;
	touch $(BUILDDIR)/.base

vpath %.$(SRCEXT) $(SRCDIR):tests
vpath %.o $(BUILDDIR)
vpath .base $(BUILDDIR)

# Compilers
CC := gcc

# Flags
OPTFLAGS := -O2 -pipe -march=armv6zk -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
DBUG := -g -Wall -Wextra -Wcast-align# -fprofile-arcs -ftest-coverage -pg

CCFLAGS := -fPIC -std=c11 $(OPTFLAGS) $(DBUG)
LDFLAGS := -fPIC -std=c11 $(OPTFLAGS) $(DBUG)

# Includes and libraries
INCLUDES := -Iinclude
LIBS := -levent -lrt -lm -ljson-c -lxxhash
CHECK_LIBS := -ljson-c -lcheck -lxxhash

# Object files to compile
MAIN := billionaire-server.o
SOURCES := $(notdir $(shell find $(SRCDIR) -type f -name *.$(SRCEXT) -not -name $(MAIN:.o=.$(SRCEXT))))
OBJECTS := $(SOURCES:.$(SRCEXT)=.o)

CHECK_BOOK := check_book.o
CHECK_CARD_LOCATION := check_card_location.o
MEM_TEST := mem_test.o

# Rules
%.o: %.$(SRCEXT) .base
	$(CC) $(CCFLAGS) $(INCLUDES) -c -o $(BUILDDIR)/$@ $<

billionaire-server: $(MAIN) $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(BINDIR)/$@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(LIBS)

check_book: $(CHECK_BOOK) book.o card_location.o command_error.o utils.o
	$(CC) $(LDFLAGS) -o $(BINDIR)/$@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(CHECK_LIBS)

check_card_location: $(CHECK_CARD_LOCATION) card_location.o command_error.o card_array.o utils.o
	$(CC) $(LDFLAGS) -o $(BINDIR)/$@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(CHECK_LIBS)

mem_test: $(MEM_TEST) $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(BINDIR)/$@ $(addprefix $(BUILDDIR)/, $(notdir $^)) $(LIBS)

check: check_book check_card_location
	$(addsuffix ;, $(addprefix ./$(BINDIR)/, $^))

clean:
	rm -rf $(BUILDDIR)/ $(BINDIR)/*

.PHONY: clean
