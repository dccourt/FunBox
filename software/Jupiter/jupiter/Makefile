# Project Name
TARGET = jupiter

USE_DAISYSP_LGPL = 1

# Needs to be SRAM to support the display - just too large otherwise.
APP_TYPE = BOOT_SRAM

# Compiler options
#OPT = -O1
OPT=-Os


# Sources
CPP_SOURCES = jupiter.cpp ../../funbox_display.cpp

# Library Locations
LIBDAISY_DIR = ../../../libDaisy
DAISYSP_DIR = ../../../DaisySP
CLOUDSEED_DIR = ../CloudSeed

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile


# Include funbox.h
C_INCLUDES += -I../../../include -I../CloudSeed \

LIBS += -lcloudseed
LIBDIR += -L../CloudSeed/build 