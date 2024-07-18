# Compiler
CC = gcc

CFLAGS = $(shell sdl2-config --cflags) $(shell pkg-config --cflags SDL2_image)

LDFLAGS = $(shell sdl2-config --ldflags) $(shell pkg-config --ldflags SDL2_image)

# Directories
SRCDIR = src
BINDIR = bin

# Files
SOURCES := $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/**/*.c)
HEADERS := $(wildcard $(SRCDIR)/*.h) $(wildcard $(SRCDIR)/**/*.h)
OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(SRCDIR)/%.o)
EXECUTABLE = $(BINDIR)/main.exe

LIBDIR = $(shell sdl2-config --libs) $(shell pkg-config --libs SDL2_image)

INCLUDE_PATHS := $(addprefix -I, $(SRCDIR)) $(addprefix -I, $(sort $(dir $(wildcard $(SRCDIR)/**/))))

# Target: compile
all: $(EXECUTABLE) clean run

# Linking
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS)  $^ $(INCLUDE_PATHS) -o $@ $(LIBDIR) -mconsole

# Compilation
$(OBJECTS): $(SRCDIR)/%.o : $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< $(INCLUDE_PATHS) -o $@ -mconsole

# Clean
clean:
	$(RM) $(OBJECTS)

run:
	./bin/main.exe