CC=gcc
TARGET=myshell
SRC=myshell.c prase.c process.c
OBJ=$(SRC:.c=.o)
LDFLAGS=

all: $(OBJ)
	$(CC) -o $(TARGET) $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJ) $(TARGET)
