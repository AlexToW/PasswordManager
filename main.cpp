/*
 *
 * */

#include <iostream>
#include <string>


#include "password_item.h"
#include "database.h"
#include "hash.h"
#include "passman.h"

int main() {
    DataBase db;
    PassMan passman(db);
    passman.Start();
    return 0;
}
