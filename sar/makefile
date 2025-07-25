include config.mk

BUILDDIR   = build
SRCDIR     = src
INCLUDEDIR = include

VERSION = $(shell git describe --tags --always)

SRC = $(shell find $(SRCDIR) -name "*.c")
OBJ = $(SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

CFLAGS += -I$(INCLUDEDIR)
CFLAGS += -DHOST='"$(HOST)"' -DPREFIX='"$(PREFIX)"' -DSYSROOT='"$(SYSROOT)"'

all : $(BUILDDIR)/sar

test : $(BUILDDIR)/sar
	@$(CC) -o $@ $^

$(BUILDDIR)/sar : $(OBJ)
	@echo '[linking into $@]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ $^ $(CFLAGS)


$(BUILDDIR)/%.o : $(SRCDIR)/%.c 
	@echo '[compiling $^]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ -c $^ $(CFLAGS)

install : all
	@echo '[installing sar]'
	@mkdir -p $(PREFIX)/bin
	@cp $(BUILDDIR)/sar $(PREFIX)/bin/sar
uninstall :
	rm -rf $(PREFIX)/bin/sar

clean :
	rm -fr build

config.mk :
	$(error run ./configure before running make)

.PHONY : all $(BUILDIR)/sar install uninstall clean
