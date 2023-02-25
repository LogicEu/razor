# razor makefile

NAME = razor

CC = gcc
STD = -std=c99
WFLAGS = -Wall -Wextra -pedantic
OPT = -O2
INC = -I.
LIB = mass fract utopia imgtool

SRCDIR = src
TMPDIR = tmp
LIBDIR = lib

SCRIPT = build.sh

SRC = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(TMPDIR)/%.o,$(SRC))
LIBS = $(patsubst %,$(LIBDIR)/lib%.a,$(LIB))
DLIB = $(patsubst %,-L%, $(LIBDIR))
DLIB += $(patsubst %,-l%, $(LIB))
INC += $(patsubst %,-I%,$(LIB))
INC += -Ispxe

DLIB += -lz -lpng -ljpeg -lglfw -lfreetype

OS=$(shell uname -s)
ifeq ($(OS),Darwin)
	DLIB += -framework OpenGL
else
	DLIB += -lGL -lGLEW -lm
endif

CFLAGS = $(STD) $(WFLAGS) $(OPT) $(INC)

$(NAME): $(OBJS) $(LIBS)
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(DLIB) $(OPNGL)

.PHONY: all clean

all: $(NAME)

$(LIBDIR)/lib%.a: %
	cd $^ && $(MAKE) && mv bin/*.a ../$(LIBDIR)

$(LIBS): | $(LIBDIR)

$(TMPDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS): $(SRC) | $(TMPDIR)

$(TMPDIR):
	mkdir -p $@

$(LIBDIR):
	mkdir -p $@

clean: $(SCRIPT)
	./$^ $@
