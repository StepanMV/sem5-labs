.PHONY: all clean run
CFLAGS = -Wall -Wextra -std=c99
TARGET = ls
SRCS = ls.c

all: $(TARGET)

$(TARGET):
	gcc $(SRCS) -o $(TARGET) $(CFLAGS)

clean:
	rm -f *.o $(TARGET)

run: $(TARGET)
	./$(TARGET)