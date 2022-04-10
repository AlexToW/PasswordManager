#pragma once

#include "database.h"
#include "hash.h"
#include "encryption.h"

class PassMan {
public:
    explicit PassMan(DataBase &db1);
    bool Registration();
    AUTHORISATION_RES Authorisation();
    void Menu();
    void Start();
    void CreatePassword();
    void FindAccountsByEmail();
    void FindPassByName();
private:
    DataBase db;
    std::string active_user;
    std::string master_pass;
};
