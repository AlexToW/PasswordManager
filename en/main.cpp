#include <iostream> 
#include <string>  
#include "encryption.h"  

int main() {	  
	std::string data = "my data";
	std::string password = "my super strong password";
	AesEncryption aes("cbc", 256);
	CryptoPP::SecByteBlock enc = aes.encrypt(data, password); 
	std::string tool = std::string(enc.begin(), enc.end());
	std::cout << tool << std::endl;
	CryptoPP::SecByteBlock dec = aes.decrypt(tool, password); 
	std::cout << std::string(dec.begin(), dec.end()) << std::endl;  
}  