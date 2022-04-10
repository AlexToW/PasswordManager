#include "hash.h"

std::string GetHash(const std::string& password, const std::string& salt) {
    std::stringstream ss;
    CryptoPP::HexEncoder encoder(new CryptoPP::FileSink(ss));

    std::string msg = password + salt;
    std::string digest;

    CryptoPP::SHA1 hash;
    hash.Update((const byte*)msg.data(), msg.size());
    digest.resize(hash.DigestSize());
    hash.Final((byte*)&digest[0]);

    CryptoPP::StringSource give_me_a_name(digest, true, new CryptoPP::Redirector(encoder));
    return ss.str();
}

std::string GenerateSalt(size_t N) {
    std::vector<char> chars = {'!',  '"',  '#',  '$',  '%',  '&',  '\'',  '(',  ')',
                               '*',  '+',  ',',  '-',  '.',  '/',  '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
                               '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',  '@',  'A',  'B',  'C',  'D',  'E',
                               'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T',
                               'U',  'V',  'W',  'X',  'Y',  'Z',  '[',  '\\',  ']',  '^',  '_',  '`',  'a',  'b',  'c',
                               'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',  'p',  'q',  'r',
                               's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '{',  '|',  '}'};
    std::string salt;
    srand(time(nullptr));
    for(size_t i = 0; i < N; i++) {
        salt += chars[rand() % chars.size()];
    }
    return salt;
}


std::pair<bool, std::vector<std::string>> GetHashWithSalt(const std::string& password) {
    std::string salt = GenerateSalt(8);
    if(1) {
        std::string hash = GetHash(password, salt);
        std::vector<std::string> v = {hash, salt};
        return std::make_pair(true, v);
    } else {
        std::vector<std::string> v = {"", salt};
        return std::make_pair(false, v);
    }
}

