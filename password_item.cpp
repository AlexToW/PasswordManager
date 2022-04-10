#include "password_item.h"

PasswordItem::PasswordItem(std::string& password, std::string& email, std::string& user_name,
                           std::string& url, std::string& app_name) {
    _password = password;
    _email = email;
    _user_name = user_name;
    _url = url;
    _app_name = app_name;
}

std::map<PassField, std::string> PasswordItem::GetPasswordItem() {
    std::map<PassField, std::string> res = {
            {PassField::PASSWORD, _password},
            {PassField::EMAIL, _email},
            {PassField::USER_NAME, _user_name},
            {PassField::URL, _url},
            {PassField::APP_NAME, _app_name}
    };
    return res;
}
