CFLAGS= -O3 -maes -msse4.1 -mpclmul -lsodium -lm
FLAGS= 
CC=gcc
TARGET=main
SRC=$(shell find *.c)
OBJS=$(subst .o,.c,$(SRC))

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(FLAGS) -c -o $@ $<

clean: 
	rm $(TARGET)