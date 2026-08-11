# ──────────────────────────────────────────────────────────────────────
# Makefile — Build system for MyShell
# ──────────────────────────────────────────────────────────────────────
# Usage:
#   make          Build the myshell binary
#   make clean    Remove build artifacts
#   make test     Run automated tests
# ──────────────────────────────────────────────────────────────────────

CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11 -g -I$(SRCDIR)
SRCDIR  = src
SRCS    = $(wildcard $(SRCDIR)/*.c)
OBJS    = $(SRCS:.c=.o)
TARGET  = myshell

# ── Default target ──────────────────────────────────────────────────
.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ── Clean ───────────────────────────────────────────────────────────
clean:
	rm -f $(OBJS) $(TARGET)

# ── Run tests ───────────────────────────────────────────────────────
test: $(TARGET)
	@chmod +x tests/test_shell.sh
	@./tests/test_shell.sh
