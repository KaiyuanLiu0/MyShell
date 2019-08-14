CC=gcc
TARGET=myshell
SRC=myshell.c prase.c process.c internal.c
OBJ=$(SRC:.c=.o)
LDFLAGS=

all: $(OBJ)
	$(CC) -o $(TARGET) $^ $(LDFLAGS)

.PHONY: clean

clean:
	$(RM) $(TARGET) $(OBJ)
