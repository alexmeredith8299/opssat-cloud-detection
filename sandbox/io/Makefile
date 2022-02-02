TARGET = dev

# Compiler options.
CC_DEV = gcc
CC_ARM = /usr/bin/arm-linux-gnueabihf-gcc

# Flags.
CFLAGS = -Wall -static -O3

# Source directory and files.
SOURCEDIR = src
HEADERS := $(wildcard $(SOURCEDIR)/*.h)
SOURCES := $(wildcard $(SOURCEDIR)/*.c)

# Target output.
BUILDTARGET = io_demo

# Target compiler environment.
ifeq ($(TARGET),arm)
	CC = $(CC_ARM)
else
	CC = $(CC_DEV)
endif

all:
	$(CC) $(CFLAGS) $(HEADERS) $(SOURCES) -o $(BUILDTARGET)

clean:
	rm -f $(BUILDTARGET)