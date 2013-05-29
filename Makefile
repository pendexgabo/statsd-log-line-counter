FLAGS=-Wall
TARGET=log-lines-counter
INCLUDE=./lib

build:
	gcc $(FLAGS) -I$(INCLUDE) log-lines-counter.c -o $(TARGET) -I/usr/local/include -L/opt/local/include -I/opt/local/include -L/usr/local/lib -R/usr/local/lib -lpcre

clean:
	rm $(TARGET)