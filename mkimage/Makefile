# Makefile

CFLAGS = -c -Wall -O2 -MMD
LDFLAGS = -fno-exceptions -s

MAKEFLAGS += -s

#ifeq ($(OS), Windows_NT)
#	CC = mingw32-gcc
#	EXE_SUFFIX = .exe
#else
	CC = gcc
#endif

LD = $(CC)

MKIMAGE_EXE = mkimage$(EXE_SUFFIX)

all: $(MKIMAGE_EXE)

MKIMAGE_SRC = \
	mkimage.c \
	crc32.c

MKIMAGE_OBJS = $(MKIMAGE_SRC:.c=.o)

RULES = $(MKIMAGE_SRC:.c=.d) 

%.o:%.c	Makefile
	@echo CC $<
	$(CC) $(CFLAGS) -c $< -o $@

$(MKIMAGE_EXE): $(MKIMAGE_OBJS)
	@echo Linking $@
	$(LD) -o $@ $^ $(LDFLAGS)

clean:
	-$(RM) $(MKIMAGE_OBJS)
	-$(RM) $(RULES)
	-$(RM) $(MKIMAGE_EXE)
	-include $(RULES)
