#********************************************************************
# File:  makefile
#--------------------------------------------------------------------
# Description:
#
# Makefile for back and restore programs
#--------------------------------------------------------------------

PROG_NAME = backup

PROJECT_DIR =  ../..
DIR =       ..
TMP_DIR =   $(DIR)

######################################################################
# Source, Object, and Binary Directories
#---------------------------------------------------------------------

MAK_DIR = $(DIR)/mak
SRC_DIR = $(DIR)/src
OBJ_DIR = $(TMP_DIR)/obj
INC_DIR = $(DIR)/inc
BIN_DIR = $(DIR)/mak

MBUTILS_DIR     = $(PROJECT_DIR)/mbUtilsLinux
MBUTILS_INC_DIR = $(MBUTILS_DIR)/inc
MBUTILS_LIB_DIR = $(MBUTILS_DIR)/mak
MBUTILS_MAC_DIR = $(MBUTILS_DIR)/mak

######################################################################
# Source Files
#---------------------------------------------------------------------

SRC_FILES = \
	$(SRC_DIR)/backup.c \
	$(SRC_DIR)/restore.c

######################################################################
# Include Files
#---------------------------------------------------------------------

INC_FILES = \
	$(INC_DIR)/backup.h

######################################################################
# Make Files
#---------------------------------------------------------------------

######################################################################
# Object Files
#---------------------------------------------------------------------

OBJ_FILES = $(SRC_FILES:$(SRC_DIR)%.c=$(OBJ_DIR)%.o)

######################################################################
# Compile Flags
#---------------------------------------------------------------------

CC =  gcc

C_FLAGS =        \
	-DLINUX=1   \
	-Wall

######################################################################
# Include paths
#---------------------------------------------------------------------

INC_DIRS =                  \
	-I$(INC_DIR)            \
	-I$(MBUTILS_INC_DIR)

######################################################################
# Libraries
#---------------------------------------------------------------------

ifeq ($(OS),fc32)
LIBS = \
	$(MBUTILS_LIB_DIR)/mbUtils.a \
	/usr/lib/libpthread.so 
else
LIBS = \
	$(MBUTILS_LIB_DIR)/mbUtils.a \
	/usr/lib64/libpthread.so
endif

######################################################################
# Targets
#---------------------------------------------------------------------

all: $(BIN_DIR)/backup $(BIN_DIR)/restore

$(BIN_DIR)/backup: $(OBJ_DIR)/backup.o $(MBUTILS_LIB_DIR)/mbUtils.a
	cd $(MBUTILS_MAC_DIR); make
	$(CC) $(C_FLAGS) $(OBJ_DIR)/backup.o $(LIBS) -o $(BIN_DIR)/backup

$(BIN_DIR)/restore: $(OBJ_DIR)/restore.o $(MBUTILS_LIB_DIR)/mbUtils.a
	cd $(MBUTILS_MAC_DIR); make
	$(CC) $(C_FLAGS) $(OBJ_DIR)/restore.o $(LIBS) -o $(BIN_DIR)/restore

$(OBJ_FILES): $(LIBS) $(INC_FILES)

######################################################################
# Clean Commands
#---------------------------------------------------------------------

clean:
	rm -f $(BIN_DIR)/$(PROG_NAME)
	rm -f $(BIN_DIR)/restore
	rm -f $(OBJ_FILES)
	rm -f $(SRC_DIR)/*~ $(INC_DIR)/*~ $(MAK_DIR)/*~

######################################################################
# How to make objects
#---------------------------------------------------------------------

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) -c $(C_FLAGS) $(INC_DIRS) -o $@ $(SRC_DIR)/$*.c

######################################################################
# End of File
#---------------------------------------------------------------------

