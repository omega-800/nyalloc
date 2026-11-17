.PHONY: all clean debug release directories test


INSTR ?= -fsanitize=address,leak,undefined,pointer-compare,pointer-subtract -fno-omit-frame-pointer

LDFLAGS ?= 

CFLAGS ?= -Wfatal-errors -Wall -Wextra -Werror -std=c99 -Wl,-z,execstack

SRCDIR ?= src
OBJDIR ?= .build
TARGETDIR ?= bin
TESTDIR ?= test
SRC := $(shell find $(SRCDIR) -type f -name *.c)
OBJ := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SRC:.c=.o))
OUT ?= $(notdir $(CURDIR))
TEST_BIN ?= test_main

ifeq ($(CC),gcc)
	CFLAGS += -D_DEFAULT_SOURCE
endif

ifeq ($(origin CC),default)
	CC = tcc
endif

all: debug

directories:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(TARGETDIR)

clean:
	rm -rf $(OBJDIR)
	rm -rf $(TARGETDIR)

-include $(OBJ:.o=.d)

$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(TARGETDIR)/$(OUT) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c 
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g $(INSTR)
debug: LDFLAGS += $(INSTR)
debug: $(OUT)

release: CFLAGS += -O3 -Ofast
release: $(OUT)

$(TESTDIR)/%.o: CFLAGS += -I$(SRCDIR)
$(TESTDIR)/%.o: $(TESTDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGETDIR)/$(TEST_BIN): $(OBJ) $(TESTDIR)/main.o
	$(CC) $(OBJ) $(TESTDIR)/main.o -o $(TARGETDIR)/$(TEST_BIN) $(LDFLAGS)

test: CFLAGS += -g $(INSTR)
test: LDFLAGS += $(INSTR)
test: directories $(TARGETDIR)/$(TEST_BIN)
	$(TARGETDIR)/$(TEST_BIN)
