#Constants

CC = gcc
CFLAGS = -Wall -ggdb 
TARGET = s_2_tt
OBJS = main.o module1.o module2.o

#Targets

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS)  $(OBJS) -o $(TARGET)
main.o : module1.o module2.o
clean: 
	rm *.o

