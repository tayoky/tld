include ../config.mk

BUILDDIR   = build
SRCDIR     = src
INCLUDEDIR = ../include

VERSION = $(shell git describe --tags --always)

SRC = $(shell find $(SRCDIR) -name "*.c")
OBJ = $(SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

CFLAGS += -I$(INCLUDEDIR)
CFLAGS += -DHOST='"$(HOST)"' -DPREFIX='"$(PREFIX)"' -DSYSROOT='"$(SYSROOT)"'

all : $(BUILDDIR)/libttc.a

$(BUILDDIR)/libttc.a : $(OBJ)
	@echo '[archiving into $@]'
	@mkdir -p $(shell dirname $@)
	@$(AR) rcs $@ $^ 


$(BUILDDIR)/%.o : $(SRCDIR)/%.c 
	@echo '[compiling $^]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ -c $^ $(CFLAGS)

clean :
	rm -fr build

config.mk :
	$(error run ./configure before running make)

.PHONY : all $(BUILDIR)/libttc.a clean
