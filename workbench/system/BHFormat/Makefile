OS=$(shell uname)
ifeq "$(OS)" "MorphOS"
STRIP	= strip --remove-section=.comment
else
STRIP   = echo >NIL:
endif
ifeq "$(OS)" "AROS"
NOIXEMUL =
else
NOIXEMUL = -noixemul
endif

CC	= gcc

CFLAGS	= -Werror-implicit-function-declaration -Wswitch -Wunused $(NOIXEMUL)

# Comment out one of these lines to select between debug and `release' versions
#CFLAGS	+= -DDEBUG=1
CFLAGS	+= -DNDEBUG -O2

Format : main.o cli.o gui.o
	$(CC) $(CFLAGS) -o Format main.o cli.o gui.o
	$(STRIP) $@

.c.o:
	$(CC) $(CFLAGS) -c $*.c

main.o cli.o gui.o : format.h

install: Format
	Copy Format SYS:System/
	Copy Format.info SYS:System/

clean:
	Delete "#?.o"
	Delete Format

