include config.mk

BUILDDIR   = build
SRCDIR     = src
INCLUDEDIR = ../include
NAME       = sar

VERSION = $(shell git describe --tags --always)

SRC = $(shell find $(SRCDIR) -name "*.c")
OBJ = $(SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

CFLAGS += -I$(INCLUDEDIR)
CFLAGS += -DHOST='"$(HOST)"' -DPREFIX='"$(PREFIX)"' -DSYSROOT='"$(SYSROOT)"'

include ../rules.mk
