all: main

main: main.o database.o passman.o hash.o encryption.o password_item.o
	g++ -o main main.o database.o passman.o hash.o encryption.o password_item.o -lcryptopp -lpqxx

main.o: main.cpp database.h passman.h
	g++ -c main.cpp

hash.o: hash.cpp hash.h
	g++ -c hash.cpp -lcryptopp

database.o: database.cpp database.h
	g++ -c database.cpp -lpqxx

password_item.o: password_item.cpp password_item.h
	g++ -c password_item.cpp

encryption.o: encryption.cpp encryption.h
	g++ -c encryption.cpp -lcryptopp