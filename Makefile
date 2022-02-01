TARGET = dev

# Compiler options.
CC_DEV = g++
CC_ARM = /usr/bin/arm-linux-gnueabihf-g++

# Header includes.
INCLUDEPATH = 

# Flags.
# use Os to optimize for storage space
CFLAGS = -Wall -static -Os

# Library flags
LIBFLAGS = 

# Source directory and files.
SOURCEDIR = src
HEADERS := $(wildcard $(SOURCEDIR)/*.h)
SOURCES := $(wildcard $(SOURCEDIR)/*.cpp)

APPNAME = test_helloworld

# Target output.
BUILDTARGET = bin/$(APPNAME).out
BUILDTARGET_ARM = bin/$(APPNAME)_arm.out

# Target compiler environment.
ifeq ($(TARGET),arm)
	CC = $(CC_ARM)
else
	CC = $(CC_DEV)
endif

all:
	$(CC) $(CFLAGS) $(INCLUDEPATH) $(HEADERS) $(SOURCES) -o $(BUILDTARGET) $(LIBFLAGS)

arm:
	$(CC_ARM) $(CFLAGS) $(INCLUDEPATH) $(HEADERS) $(SOURCES) -o $(BUILDTARGET_ARM) $(LIBFLAGS)

clean:
	rm -f $(BUILDTARGET)