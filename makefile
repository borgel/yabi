INC_DIR = include/
OBJECTS = yabi.o
CFLAGS = -g -Wall -Wpedantic -I$(INC_DIR)

.PHONY: all clean

all: yabi.a tester

%.o: src/%.c
	gcc $(CFLAGS) -c $< -o $@

%.o: test/%.c
	gcc $(CFLAGS) -c $< -o $@

yabi.a: $(OBJECTS)
	ar rcs $@ $^

tester: yabi.a main.o
	gcc $(CFLAGS) $^ -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f yabi.a

	-rm -f tester
	-rm -f test/*.o

