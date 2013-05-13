FLAGS=-Wall
TARGET=log-lines-counter
INCLUDE=./lib

build:
	gcc $(FLAGS) -I$(INCLUDE) log-lines-counter.c -o $(TARGET)

clean:
	rm $(TARGET)