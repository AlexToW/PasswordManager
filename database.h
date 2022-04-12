#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <pqxx/pqxx>

#include "password_item.h"

enum class AUTHORISATION_RES {
    FAIL,
    SUCCESS,
    NO_USER
};

class DataBase {
public:
    DataBase(std::string const & pass_db);
    bool ExistDB();
    bool ExistTables();
    std::string GetConnInfo();
    bool CreateDB();
    bool CreateTables();
    bool InsertPassword(PasswordItem& p);
    std::pair<bool, std::vector<std::pair<std::string, std::string>>>
    SelectEmailAll(const std::string& user, const std::string& email);
    std::pair<bool, std::string> FindPass(const std::string& user, const std::string& name);
    bool InsertRegister(std::string& login, std::string& master_pass_hash, std::string& salt);
    AUTHORISATION_RES CheckAuthorisation(std::string& login, std::string& master_pass);
private:
    std::string db_name;
    std::string user_name;
    std::string password_db;
};