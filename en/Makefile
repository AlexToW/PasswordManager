all: main  

main: main.o encryption.o 
	g++ -o main main.o encryption.o -lcryptopp

main.o: main.cpp encryption.h
	g++ -c main.cpp 

encryption.o: encryption.cpp encryption.h 
	g++ -c encryption.cpp  


