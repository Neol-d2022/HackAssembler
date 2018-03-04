CC=gcc

CFLAGS=-Wall -Wextra -g3
LFLAGS=

OBJS=main.o
DEPS=
LIBS=-lm

BIN=assembler

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJS) $(BIN)

test:
	./assembler
