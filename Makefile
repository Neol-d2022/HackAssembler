CC=gcc

CFLAGS=-Wall -Wextra -Ofast
LFLAGS=-s

OBJS=main.o parser.o code.o symboltable.o avl_tree.o
DEPS=parser.h code.h symboltable.h avl_tree.h
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
