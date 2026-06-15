CFLAGS= -O3 -maes -msse4 -mpclmul -lsodium -lm -march=native
FLAGS= 
CC=gcc
TARGET=main
SRC=$(shell find . -name "*.c")
OBJS=$(subst .c,.o,$(SRC))


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	rm $(TARGET) $(OBJS)