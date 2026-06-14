CFLAGS= -O3 -maes -msse4.1
CC=gcc
TARGET=main

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET)

clean: 
	rm $(TARGET)