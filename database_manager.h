#pragma once

#include <iostream>
#include <tuple>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include "password_item.h"
#include "encryption.h"
//#include "cionout.h"
class DataBase {
public:
    DataBase();
    DataBase(const std::string& db_name);
    DataBase(const std::string& db_name, const std::string& table_name, const std::string user_name);
    bool InsertPasswordItem(PasswordItem& item);
    std::pair<bool, std::vector<std::pair<std::string, std::string>>>
        SelectEmailAll(const std::string& email);
    std::pair<bool, std::string> FindPass(const std::string& name);
    std::string GetDBconnectionInfo();
    std::string GetDBName();
    std::string GetTableName();
private:
    std::string _user_name;
    std::string _db_name;
    std::string _table_name;
};
