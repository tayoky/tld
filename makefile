include config.mk

BUILDDIR   = build
SRCDIR     = src
INCLUDEDIR = include

VERSION = $(shell git describe --tags --always)

SRC = $(shell find $(SRCDIR) -name "*.c")
OBJ = $(SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

CFLAGS += -I$(INCLUDEDIR)

all : $(BUILDDIR)/tld

test : $(BUILDDIR)/tld
	@$(CC) -o $@ $^

$(BUILDDIR)/tld : $(OBJ)
	@echo '[linking into $@]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ $^


$(BUILDDIR)/%.o : $(SRCDIR)/%.c 
	@echo '[compiling $^]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ -c $^ $(CFLAGS)

install : all
	@echo '[installing tld]'
	@mkdir -p $(PREFIX)/bin
	@cp $(BUILDDIR)/tld $(PREFIX)/bin/tld

uninstall :
	rm -f $(PREFIX)/lib/tld

clean :
	rm -fr build

config.mk :
	$(error run ./configure before running make)

.PHONY : all $(BUILDIR)/tld install uninstall clean
