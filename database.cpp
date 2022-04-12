#include "database.h"

bool DataBase::InsertRegister(std::string& login, std::string& master_pass_hash, std::string& salt) {
    /* inserts new user data into authorisation table */
    std::stringstream ss;
    ss << "INSERT INTO authorisation VALUES ('" << login << "', '" << master_pass_hash << "', '"
       << salt << "');";
    try {
        pqxx::connection C("user = postgres password = " + password_db + " dbname = passman");
        if(!C.is_open()) {
            std::cout << "Failed connection to database!" << std::endl;
            return false;
        }
        pqxx::work w(C);
        w.exec(ss.str());
        w.commit();
        C.disconnect();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

bool DataBase::InsertPassword(PasswordItem &p) {
    std::string conn_info = "user = postgres password = " + password_db + " dbname = passman";
    auto pass = p.GetPasswordItem();
    std::stringstream ss_req;
    ss_req << "INSERT INTO all_passwords (password, email, user_name, url, app_name) VALUES ("
           << "'" << pass[PassField::PASSWORD]     << "', "
           << "'" << pass[PassField::EMAIL]        << "', "
           << "'" << pass[PassField::USER_NAME]    << "', "
           << "'" << pass[PassField::URL]          << "', "
           << "'" << pass[PassField::APP_NAME]     << "');";
    std::string request_str = ss_req.str();
    try {
        pqxx::connection C(conn_info);
        if(!C.is_open()) {
            std::cout << "Failed connection to database!" << std::endl;
            return false;
        }
        pqxx::work w(C);
        // executing INSERT request
        w.exec(request_str);
        w.commit();
        C.disconnect();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

std::pair<bool, std::vector<std::pair<std::string, std::string>>>
DataBase::SelectEmailAll(const std::string& user, const std::string& email) {
    // Finds all sites and apps connected to an email. Returns pairs <url, app_name>
    // making SELECT request string
    std::stringstream ss_req;
    ss_req << "SELECT * FROM all_passwords WHERE email = '" << email << "' AND user_name = '" << user << "';";
    std::vector<std::pair<std::string, std::string>> result_v;
    // connection to database
    try {
        pqxx::connection C("user = postgres password = " + password_db + " dbname = passman");
        pqxx::work w(C);
        pqxx::result res = w.exec(ss_req.str());
        w.commit();
        if (res.empty()) {
            std::cout << "There are no any passwords in all_passwords!" << std::endl;
            std::vector<std::pair<std::string, std::string>> v = {std::make_pair("", "")};
            return std::make_pair(false, v);
        }

        for (pqxx::row row : res){
            std::pair<std::string, std::string> p =
                    std::make_pair(row[3].c_str(), row[4].c_str());
            result_v.push_back(p);
        }
        C.disconnect();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::vector<std::pair<std::string, std::string>> v = {std::make_pair("", "")};
        return std::make_pair(false, v);
    }
    return std::make_pair(true, result_v);
}

std::pair<bool, std::string> DataBase::FindPass(const std::string& user, const std::string& name) {
    /* Searches for a password by the 'name' in the url and app_name fields
        for current user */
    // making SELECT request string
    std::stringstream ss_req;
    ss_req << "SELECT * FROM all_passwords WHERE user_name = '" << user << "' AND url = '" << name
           << "' OR user_name = '" << user << "' AND app_name = '" << name << "';";
    std::pair<bool, std::string> res_p; // result to return
    //connection to database
    try {
        pqxx::connection C("user = postgres password = " + password_db + " dbname = passman");
        pqxx::work w(C);
        pqxx::result res = w.exec(ss_req.str());
        w.commit();
        if(res.empty()) {
            std::cout << "There are no any passwords found by '" << name << "'!" << std::endl;
            res_p.first = false;
            res_p.second = "";
            return res_p;
        }
        res_p = std::make_pair(true, res[0][0].c_str());
        C.disconnect();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        res_p.first = false;
        res_p.second = "";
        return res_p;
    }
    return res_p;
}

bool DataBase::CreateTables() {
    /* returns True, if tables authorisation and all_passwords were successfully created */
    std::string conn_info = "user = postgres password = " + password_db + " dbname = passman";
    std::string q1 = "create table if not exists authorisation (login varchar (300) NOT NULL UNIQUE, "
                     "password varchar (300) NOT NULL, salt varchar(100) NOT NULL UNIQUE);";
    std::string q2 = "CREATE TABLE IF NOT EXISTS all_passwords (password varchar (500) NOT NULL, "
                     "email varchar (100) NOT NULL, user_name varchar (100) NOT NULL, "
                     "url varchar (100) NOT NULL, app_name varchar (100) NOT NULL);";
    try {
        pqxx::connection C(conn_info);
        if(!C.is_open()) {
            std::cout << "Failed connection to DB 'passman'!" << std::endl;
            return false;
        }
        pqxx::nontransaction N(C);
        N.exec(q1);
        N.exec(q2);
        N.commit();
        C.disconnect();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

bool DataBase::CreateDB() {
    std::string conn_info = "user = postgres password = " + password_db + " dbname = template1";
    std::string q = "create database passman;";
    try {
        pqxx::connection C(conn_info);
        if(!C.is_open()) {
            std::cout << "Failed connection to DB!" << std::endl;
            return false;
        }
        pqxx::nontransaction N(C);
        N.exec(q);
        N.commit();
        C.disconnect();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

DataBase::DataBase(std::string const & pass_db) {
    password_db = pass_db;
    db_name = "template1";
    user_name = "postgres";
    if(!ExistDB()) {
        // creating DB passman and tables authorisation, all_passwords
        if(!CreateDB() || !CreateTables()) {
            std::cout << "DataBase error!" << std::endl;
            exit(1);
        }
    } else if(!ExistTables()) {
        // creating tablex authorisation, all_passwords
        if(!CreateTables()) {
            std::cerr << "DataBase error!" << std::endl;
            exit(1);
        }
    }
}

bool DataBase::ExistDB() {
    std::string q = "select * from pg_database where datname = 'passman';";
    std::string conn = "user = postgres dbname = template1";
    pqxx::result r;
    try {
        pqxx::connection C(conn);
        if(!C.is_open()) {
            std::cout << "Failed open DB!" << std::endl;
            exit(1);
        }
        pqxx::nontransaction N(C);
        r = N.exec(q);
        N.commit();
        C.disconnect();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    return !r.empty();
}

bool DataBase::ExistTables() {
    std::string conn_info = "user = postgres dbname = passman";
    std::string q1 = "SELECT EXISTS(SELECT 1 FROM information_schema.tables \n"
                     "              WHERE table_catalog='passman' AND \n"
                     "                    table_schema='public' AND \n"
                     "                    table_name='all_passwords');";
    std::string q2 = "SELECT EXISTS(SELECT 1 FROM information_schema.tables \n"
                     "              WHERE table_catalog='passman' AND \n"
                     "                    table_schema='public' AND \n"
                     "                    table_name='authorisation');";
    pqxx::result r1, r2;
    try {
        pqxx::connection C(conn_info);
        if(!C.is_open()) {
            std::cout << "Failed open DB!" << std::endl;
            exit(1);
        }
        pqxx::nontransaction N(C);
        r1 = N.exec(q1);
        r2 = N.exec(q2);
        N.commit();
        C.disconnect();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    return strcmp(r1[0][0].c_str(), "t") == 0 && strcmp(r2[0][0].c_str(), "t") == 0;
}

std::string DataBase::GetConnInfo() {
    return "user = " + user_name + " dbname = " + db_name;
}

