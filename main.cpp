
#include <iostream>
#include <string>

#include "database.h"
#include "passman.h"

int main() {
    DataBase db;
    PassMan passman(db);
    passman.Start();
    return 0;
}
