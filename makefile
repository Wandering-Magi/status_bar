CC = gcc
CFLAGS = -std=c23 -Wall -lm -Include
TARGET = build/mybar
SRC = src/main.c src/network.c src/clock.c src/helper.c
OBJ = $(SRC:src/%.c=obj/%.o)
DEPS = include/constants.h include/network.h include/clock.h include/helper.h

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p build
	$(CC) -o $@ $^

obj/%.o: src/%.c $(DEPS)
	@mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf obj build
