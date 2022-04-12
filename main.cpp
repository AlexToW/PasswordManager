
#include <iostream>
#include <string>

#include "database.h"
#include "passman.h"

int main(int argc, char* argv[]) {
    if(argc < 2) {
        throw std::runtime_error("To few arguments: expected 2!");
    }
    DataBase db(std::string(argv[1], sizeof(argv[1])));
    PassMan passman(db);
    passman.Start();
    return 0;
}
