UNAME_S := $(shell uname -s)

CC      := clang
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -Wshadow \
            -Wstrict-prototypes -Wmissing-prototypes \
            -fstack-protector-strong
LDFLAGS :=
TARGET  := build/frogger
SRCDIR  := src
INCDIR  := include
OBJDIR  := build/obj

SRCS    := $(wildcard $(SRCDIR)/*.c)
OBJS    := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -lncurses
else
    LDFLAGS += -lncurses -lm
endif

# ── Targets ───────────────────────────────────────────────────────────────

.PHONY: all clean debug release asan run

all: debug

debug: CFLAGS += -g3 -O0 -DDEBUG
debug: $(TARGET)

release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

asan: CC := clang
asan: CFLAGS  += -g3 -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
asan: LDFLAGS += -fsanitize=address,undefined
asan: $(TARGET)

$(TARGET): $(OBJS) | build
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

build:
	mkdir -p build

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf build/

run: debug
	./$(TARGET)
