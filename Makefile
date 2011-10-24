#Constants

CC = gcc
CXX = g++
CXXFLAGS = -Wall  
CFLAGS = $(CXXFLAGS)  
TARGET = s_2_tt
OBJS = main.o module1.o 
OBJSXX = module2.o

#Targets

$(TARGET): $(OBJS) $(OBJSXX)
	$(CXX) $(CXXFLAGS) module1.o module2.o main.o -o $(TARGET)

$(OBJSXX): module1.o

main.o : $(OBJSXX)
clean: 
	rm *.o

