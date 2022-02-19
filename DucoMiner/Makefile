OBJS	= DucoMiner.o Logger.o main.o PickAndShowel.o
SOURCE	= DucoMiner.cpp Logger.cpp main.cpp PickAndShowel.cpp
HEADER	= AbstractMiner.h DucoMiner.h json.h Logger.h PickAndShowel.h
OUT	= DucoMiner
CC	 = g++
FLAGS	 =  -c -Ofast -Wall
LFLAGS	 = -lssl -lcrypto -lpthread

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

DucoMiner.o: DucoMiner.cpp
	$(CC) $(FLAGS) DucoMiner.cpp -std=c++17

Logger.o: Logger.cpp
	$(CC) $(FLAGS) Logger.cpp -std=c++17 

main.o: main.cpp
	$(CC) $(FLAGS) main.cpp -std=c++17

PickAndShowel.o: PickAndShowel.cpp
	$(CC) $(FLAGS) PickAndShowel.cpp -std=c++17  -lssl -lcrypto


clean:
	rm -f $(OBJS) $(OUT)
