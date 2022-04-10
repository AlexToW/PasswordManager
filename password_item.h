#pragma once
#include <string>
#include <map>

enum class PassField {
    PASSWORD,
    EMAIL,
    USER_NAME,
    URL,
    APP_NAME
};

class PasswordItem {
public:
    PasswordItem(std::string& password, std::string& email, std::string& user_name,
                 std::string& url, std::string& app_name);
    std::map<PassField, std::string> GetPasswordItem();

private:
    std::string _password;
    std::string _email;
    std::string _user_name;
    std::string _url;
    std::string _app_name;
};
