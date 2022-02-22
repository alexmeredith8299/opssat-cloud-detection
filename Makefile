cTARGET = dev

# Compiler options.
CC_DEV = g++
CC_ARM = /usr/bin/arm-linux-gnueabihf-g++

# Header includes.
INCLUDEPATH = include -Iinclude/csv2/include

# Flags.
# use Os to optimize for storage space
CFLAGS = -Wall -static -Os

# Library flags
LIBFLAGS =

# Source directory and files.
SOURCEDIR = src
LUM_HEADERS := $(wildcard $(SOURCEDIR)/*.h)
LUM_SOURCES := $(wildcard $(SOURCEDIR)/main.cpp)

RF_HEADERS := $(wildcard $(SOURCEDIR)/*.h)
RF_SOURCES := $(wildcard $(SOURCEDIR)/main_rf.cpp)

LUM_APPNAME = opssat_segment
RF_APPNAME = opssat_rf_segment

# Target output.
LUM_BUILDTARGET = bin/$(LUM_APPNAME).out
LUM_BUILDTARGET_ARM = bin/$(LUM_APPNAME)_arm.out

RF_BUILDTARGET = bin/$(RF_APPNAME).out
RF_BUILDTARGET_ARM = bin/$(RF_APPNAME)_arm.out

# Target compiler environment.
ifeq ($(TARGET),arm)
	CC = $(CC_ARM)
else
	CC = $(CC_DEV)
endif

all:
	$(CC) $(CFLAGS) -I$(INCLUDEPATH) $(LUM_HEADERS) $(LUM_SOURCES) -o $(LUM_BUILDTARGET) $(LIBFLAGS)
	$(CC) $(CFLAGS) -I$(INCLUDEPATH) $(RF_HEADERS) $(RF_SOURCES) -o $(RF_BUILDTARGET) $(LIBFLAGS)

arm:
	$(CC_ARM) $(CFLAGS) -I$(INCLUDEPATH) $(LUM_HEADERS) $(LUM_SOURCES) -o $(LUM_BUILDTARGET_ARM) $(LIBFLAGS)
	$(CC_ARM) $(CFLAGS) -I$(INCLUDEPATH) $(RF_HEADERS) $(RF_SOURCES) -o $(RF_BUILDTARGET) $(LIBFLAGS)

clean:
	rm -f $(LUM_BUILDTARGET)
	rm -f $(RF_BUILDTARGET)
