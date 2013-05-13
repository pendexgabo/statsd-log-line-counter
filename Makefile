FLAGS=-Wall
TARGET=log-lines-counter
build:
	gcc $(FLAGS) log-lines-counter.c -o $(TARGET)

clean:
	rm $(TARGET)