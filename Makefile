CC = gcc

CFLAGS = -Wno-implicit-function-declaration -g -MD
LFLAGS = -lreadline
INC = -Iinclude

SRCS = ${wildcard src/*.c}
OBJS = ${patsubst src/%.c, obj/%.o, ${SRCS}}
DEPENDS = ${wildcard include/*.h}
DEPS := ${OBJS:.o=.d}

-include ${DEPS}

.PHONY: clean all exec test doitall

shell: $(OBJS)
	$(CC) $^ -o bin/$@
	cp ./bin/shell ./
	rm ./bin/shell

obj/%.o: src/%.c
	$(CC) $(CFLAG) $(INC) -c -o $@ $<

# .PHONY
all:
	shell

clean:
	rm -f obj/*.o obj/*.d

exec:
	./shell

test:
	./bin/number

doitall:
	make clean
	make
	make exec
	make clean
	rm ./shell