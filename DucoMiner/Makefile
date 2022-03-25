OBJS    = format.o Logger.o main.o PickAndShowel.o DucoMiner.o
SOURCE  = format.cc Logger.cpp main.cpp PickAndShowel.cpp DucoMiner.cpp
HEADER  = AbstractMiner.h DucoMiner.h fmt/core.h fmt/format.h fmt/format-inl.h json.h Logger.h PickAndShowel.h
OUT     = DucoMiner
CC       = g++
FLAGS    = -c -Ofast -Wall
LFLAGS   = -lssl -lcrypto -lpthread

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

format.o: format.cc
	$(CC) $(FLAGS) format.cc -std=c++17

Logger.o: Logger.cpp
	$(CC) $(FLAGS) Logger.cpp -std=c++17

main.o: main.cpp
	$(CC) $(FLAGS) main.cpp -std=c++17

PickAndShowel.o: PickAndShowel.cpp
	$(CC) $(FLAGS) PickAndShowel.cpp -std=c++17 -lssl -lcrypto

DucoMiner.o: DucoMiner.cpp
	$(CC) $(FLAGS) DucoMiner.cpp -std=c++17


clean:
	rm -f $(OBJS) $(OUT)
