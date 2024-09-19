.PHONY: all clean run
TARGET = ls
SRCS = ls.c

all:
	gcc $(SRCS) -o $(TARGET)

clean:
	rm -f *.o $(TARGET)

run: $(TARGET)
	./$(TARGET)
