CC = gcc
CFLAG = -Wno-implicit-function-declaration -g -MD
LFLAGS = -lreadline
INC = -Iinclude

SRCS = $(wildcard src/*.c)

OBJS = $(pathsubst src/%.c, obj/%.c, ${SRCS})

.PHONY: depend clean all
DEPS = $(OBJS:.o=.d)

all: shell

-include $(DEPS)

shell: $(OBJS)
	$(CC) $^ -o bin/$@ $(LFLAGS)

obj/%.o: src/%.c
	$(CC) $(CFLAG) $(INC) -c -o $@ $<

clean:
	@rm -f bin/* obj/*.o obj/*.d