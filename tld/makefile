include config.mk

BUILDDIR   = build
SRCDIR     = src
INCLUDEDIR = include ../include
LDDIR      = scripts

VERSION = $(shell git describe --tags --always)

SRC = $(shell find $(SRCDIR) -name "*.c")
OBJ = $(SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

CFLAGS += $(foreach DIR, $(INCLUDEDIR), -I$(DIR))
CFLAGS += -DHOST='"$(HOST)"' -DPREFIX='"$(PREFIX)"' -DSYSROOT='"$(SYSROOT)"'

all : $(BUILDDIR)/tld

test : $(BUILDDIR)/tld
	@$(CC) -o $@ $^

$(BUILDDIR)/tld : $(OBJ)
	@echo '[linking into $@]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ $^ $(CFLAGS) -L../libttc/build -lttc


$(BUILDDIR)/%.o : $(SRCDIR)/%.c 
	@echo '[compiling $^]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ -c $^ $(CFLAGS)

install : all
	@echo '[installing tld]'
	@mkdir -p $(PREFIX)/bin $(PREFIX)/lib/tld/
	@cp $(BUILDDIR)/tld $(PREFIX)/bin/tld
	@cp $(LDDIR)/*.ld $(PREFIX)/lib/tld
uninstall :
	rm -rf $(PREFIX)/bin/tld $(PREFIX)/lib/tld

clean :
	rm -fr build

config.mk :
	$(error run ./configure before running make)

.PHONY : all $(BUILDIR)/tld install uninstall clean
