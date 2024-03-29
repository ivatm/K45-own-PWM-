#We try to detect the OS we are running on, and adjust commands as needed
ifeq ($(OSTYPE),cygwin)
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=.out
else ifeq ($(OS),Windows_NT)
	CLEANUP = rm
	MKDIR = mkdir
	TARGET_EXTENSION=.exe
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=.out
endif

#Path Definitions
PATHU = ../unity/
PATHS = ./
PATHS1 = ../src/
PATHS2 = ../src/ADC/
PATHT = ./
PATHI = ../inc/
PATHB = build/


#determine our source files
SRCU = $(PATHU)unity.c
SRCS = $(wildcard $(PATHS)*.c)
SRCS1 = $(wildcard $(PATHS1)globalvardefs.c)
SRCS2 = $(wildcard $(PATHS2)ADC_Unit.c)
SRCT = $(wildcard $(PATHT)*.c)
SRC = $(SRCU) $(SRCS) $(SRCS2) $(SRCT)


#Files We Are To Work With
OBJU = $(patsubst $(PATHU)%.c,$(PATHB)%.o,$(SRCU))
OBJS1 = $(patsubst $(PATHS1)%.c,$(PATHB)%.o,$(SRCS1))
OBJS2 = $(patsubst $(PATHS2)%.c,$(PATHB)%.o,$(SRCS2))
OBJT = $(patsubst $(PATHT)%.c,$(PATHB)%.o,$(SRCT))
OBJ = $(OBJU) $(OBJS1) $(OBJS2) $(OBJT)

#Other files we care about
DEP = $(PATHU)unity.h $(PATHU)unity_internals.h
TGT = $(PATHB)test$(TARGET_EXTENSION)

#Tool Definitions
CC=gcc
CFLAGS=-I. -I$(PATHU) -I$(PATHI) -DTEST

$(PATHB)%.o:: $(PATHS1)%.c $(DEP)
	$(CC) -c $(CFLAGS) $< -o $@

$(PATHB)%.o:: $(PATHS2)%.c $(DEP)
	$(CC) -c $(CFLAGS) $< -o $@

$(PATHB)%.o:: $(PATHT)%.c $(DEP)
	$(CC) -c $(CFLAGS) $< -o $@

$(PATHB)%.o:: $(PATHU)%.c $(DEP)
	$(CC) -c $(CFLAGS) $< -o $@

$(TGT): $(OBJ)
	gcc -o $@ $^

clean:
	$(CLEANUP) -f $(PATHB)*.o
	$(CLEANUP) -f $(TGT)
	$(CLEANUP) -r $(PATHB)

test: $(PATHB) $(TGT)
	./$(TGT)

$(PATHB):
	$(MKDIR) $(PATHB)

all: clean test

.PHONY: all
.PHONY: clean
.PHONY: test
