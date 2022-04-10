#include "passman.h"


void PassMan::CreatePassword() {
    std::string password, email, user_name, url, app_name;
    user_name = active_user;
    std::cout << "Enter email: ";
    std::cin >> email;
    std::cout << "Enter app_name: ";
    std::cin >> app_name;
    std::cout << "Enter URL: ";
    std::cin >> url;
    std::cout << "Enter password: ";
    std::cin >> password;
    // encrypt password
    AesEncryption aes("cbc", 256);
    CryptoPP::SecByteBlock enc = aes.encrypt(password, master_pass);
    std::string encrypted_pass = std::string(enc.begin(), enc.end());

    PasswordItem pass(encrypted_pass, email, user_name,
                      url, app_name);
    bool inserted = db.InsertPassword(pass);
    if(!inserted) {
        std::cerr << "Create new password failed!" << std::endl;
    } else {
        std::cout << "Password successfully saved!" << std::endl;
    }
}

PassMan::PassMan(DataBase &db1) : db(db1) {}

void PassMan::FindAccountsByEmail() {
    std::string email;
    std::cout << "Enter email: ";
    std::cin >> email;
    auto res = db.SelectEmailAll(active_user, email);
    if(!res.first) {
        std::cout << "Failed!" << std::endl;
    } else {
        for(const auto& item : res.second) {
            std::cout << item.first << "\t" << item.second << std::endl;
        }
    }
}

void PassMan::FindPassByName() {
    std::string name;
    std::cout << "Enter url or app_name: ";
    std::cin >> name;
    auto res = db.FindPass(active_user, name);
    bool found = res.first;
    if(found) {
        // decrypt password
        std::string password_encrypted = res.second;
        AesEncryption aes("cbc", 256);
        CryptoPP::SecByteBlock dec = aes.decrypt(password_encrypted, master_pass);
        std::string password_decrypted = std::string(dec.begin(), dec.end());
        std::cout << "Password for " << name << ": " << password_decrypted << std::endl;
    } else {
        std::cout << "There are no any passwords for " << name << std::endl;
    }
}

bool CheckRegistrationUser(std::string& login) {
    // returns True, if 'login' hasn't been used yet
    std::string q = "SELECT * FROM authorisation WHERE login = '" + login + "';";
    pqxx::result r;
    try {
        pqxx::connection C("user = postgres dbname = passman");
        if(!C.is_open()) {
            std::cout << "Failed open DB!" << std::endl;
            exit(1);
        }
        pqxx::nontransaction N(C);
        r = N.exec(q);
        N.commit();
        C.disconnect();
    }  catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    return r.empty();
}

bool PassMan::Registration() {
    std::string login;
    std::cout << "Enter user name: ";
    std::cin >> login;
    // check that the given login is not used yet
    bool not_used = CheckRegistrationUser(login);
    if(!not_used) {
        std::cout << "User with name '" << login
                  << "' already exists. Please, log in or choose another login!" << std::endl;
        return false;
    }

    std::string master_pass1, master_pass2;
    std::cout << "Create Master Password: ";
    std::cin >> master_pass1;
    if(master_pass1.length() < 8) {
        std::cout << "The Master Password cannot be shorter than 8 characters!" << std::endl;
        return false;
    }
    std::cout << "Confirm Master Password: ";
    std::cin >> master_pass2;
    if(master_pass1 != master_pass2) {
        std::cout << "The entered passwords do not match!" << std::endl;
        return false;
    }

    auto hash = GetHashWithSalt(master_pass1);
    if(hash.first) {
        bool flag = db.InsertRegister(login, hash.second[0], hash.second[1]);
        if(flag) {
            master_pass = master_pass1;
            return true;
        }
    }
    return false;
}

AUTHORISATION_RES DataBase::CheckAuthorisation(std::string& login, std::string& master_pass) {
    std::string q = "SELECT * FROM authorisation WHERE login = '" + login + "';";
    pqxx::result r;
    try {
        pqxx::connection C("user = postgres dbname = passman");
        if(!C.is_open()) {
            std::cout << "Failed connection to DB 'passman'!" << std::endl;
            return AUTHORISATION_RES::FAIL;
        }
        pqxx::nontransaction N(C);
        r = N.exec(q);
        N.commit();
        C.disconnect();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    if(r.empty()) {
        return AUTHORISATION_RES::NO_USER;
    }
    std::string pass_hash_db = r[0][1].c_str(), salt_db = r[0][2].c_str();
    std::string new_hash = GetHash(master_pass, salt_db);
    return (pass_hash_db == new_hash) ? AUTHORISATION_RES::SUCCESS : AUTHORISATION_RES::FAIL;
}

AUTHORISATION_RES PassMan::Authorisation() {
    std::string login, master_pass_entered;
    std::cout << "Enter login: ";
    std::cin >> login;
    std::cout << "Enter Master Password: ";
    std::cin >> master_pass_entered;
    master_pass = master_pass_entered;
    auto res = db.CheckAuthorisation(login, master_pass_entered);
    if(res == AUTHORISATION_RES::SUCCESS) {
        active_user = login;
    }
    return res;
}


void PassMan::Start() {
    std::cout << "_________Password Manager_________" << std::endl;
    std::cout << "Registration / Authorisation / Exit? [r/a/e]: ";
    std::string command;
    std::cin >> command;
    if(command == "e") {
        exit(0);
    }
    if(command == "r") {
        if(Registration()) {
            std::cout << "Registration completed successfully." << std::endl;
            std::cout << "Please login using the created account." << std::endl;
            Start();
        } else {
            std::cout << "Registration failed!" << std::endl;
            Start();
        }
    } else if(command == "a") {
        auto aut_res = Authorisation();
        if(aut_res == AUTHORISATION_RES::SUCCESS) {
            std::cout << "You are in!" << std::endl;
            Menu();
        } else if(aut_res == AUTHORISATION_RES::FAIL) {
            std::cout << "Authorisation failed!" << std::endl;
            Start();
        } else {
            std::cout << "There are no any accounts with this login!" << std::endl;
            Start();
        }
    } else {
        std::cout << "Choose one of the proposed options!" << std::endl;
        Start();
    }
}

void PassMan::Menu() {
    std::cout << std::string(20, '_') << std::endl;
    std::cout << std::string(8, '_') << "Menu" << std::string(8, '_') << std::endl;
    std::cout << "1. Create new password" << std::endl;
    std::cout << "2. Find all sites and apps connected to an email" << std::endl;
    std::cout << "3. Find a password for a site or app" << std::endl;
    std::cout << "4. Exit" << std::endl;
    std::cout << std::string(20, '_') << std::endl;
    std::cout << ": ";
    std::string answer;
    std::cin >> answer;
    if(answer == "1") {
        CreatePassword();
    } else if(answer == "2") {
        FindAccountsByEmail();
    } else if(answer == "3") {
        FindPassByName();
    } else if(answer == "4") {
        exit(0);
    } else {
        std::cout << "Choose one of the suggested options!" << std::endl;
    }
    Menu();
}

