include config.mk

BUILDDIR   = build
SRCDIR     = src
INCLUDEDIR = include ../include
LDDIR      = scripts
NAME       = tld

VERSION = $(shell git describe --tags --always)

SRC = $(shell find $(SRCDIR) -name "*.c")
OBJ = $(SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

CFLAGS += $(foreach DIR, $(INCLUDEDIR), -I$(DIR))
CFLAGS += -DHOST='"$(HOST)"' -DPREFIX='"$(PREFIX)"' -DSYSROOT='"$(SYSROOT)"'

include ../rules.mk
