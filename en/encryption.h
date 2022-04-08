#include <string>
#include <map>
#include <stdexcept>
#include <fstream>
#include <memory>

#include <cryptopp/aes.h>
#include <cryptopp/modes.h>


// The encryption and authentication keys
///   are derived from the supplied key or password using HKDF / PBKDF2.
///   The key can be set either with `setMasterKey` or with `randomKeyGen`.
///   Encrypted data format: salt[16] + iv[16] + ciphertext[n] + mac[32].
///   Ciphertext authenticity is verified with HMAC SHA256.
class AesEncryption
{
public:
    unsigned int keyIterations = 20000;
    // accepts and returns Base64 encoded data.
    bool base64 = true;
    //Identifies encryption mode.
    static const enum { CBC = 10, CFB = 20 } Mode;
    static const enum { ENCRYPT = 1, DECRYPT = 2 } Method;

    // throw runtime_error When the mode is not supported or size is invalid.
    AesEncryption(std::string mode = "CBC", unsigned int size = 128);

    // Encrypts data using the supplied password.
    CryptoPP::SecByteBlock encrypt(CryptoPP::SecByteBlock data, CryptoPP::SecByteBlock password);

    // Encrypts data using the supplied password.
    CryptoPP::SecByteBlock encrypt(std::string data, std::string password); 
    // Decrypts data using the supplied password.
    CryptoPP::SecByteBlock decrypt(CryptoPP::SecByteBlock data, CryptoPP::SecByteBlock password);
    CryptoPP::SecByteBlock decrypt(std::string data, std::string password);
protected:
    // Handles exceptions (prints the error message by default).
    //exception The exception object.
    void errorHandler(const std::exception& exception);
private:
    // the supported AES modes.
    const std::map<std::string, unsigned int> modes = {
        { "CBC", AesEncryption::CBC }, { "CFB", AesEncryption::CFB }
    };
    //Holds the AES key sizes.
    const int sizes[3] = { 128, 192, 256 };
    //The salt size in bytes.
    static const int saltLen = 16;
    //The IV size in bytes.
    static const int ivLen = 16;
    //The MAC size in bytes.
    static const int macLen = 32;
    //The HMAC key size in bytes.
    static const int macKeyLen = 32;

    //he selected AES mode.
    std::string mode;
    //The key size in bytes.
    size_t keyLen;

    //Creates random bytes; used for IV, salt and key generation.
    CryptoPP::SecByteBlock randomBytes(size_t size);

    //Derives encryption and authentication keys from a key or password.
    void keys(
        CryptoPP::SecByteBlock password, const unsigned char* salt,
        CryptoPP::SecByteBlock& aesKey, CryptoPP::SecByteBlock& macKey
    );

    //Creates a SymmetricCipher object; used for encryption / decryption.
    //The encryption mode (ENCRYPT / DECRYPT).
    std::shared_ptr<CryptoPP::SymmetricCipher> cipher(
        CryptoPP::SecByteBlock key, CryptoPP::SecByteBlock iv, unsigned int mode);

    //Computes the MAC of ciphertext; used for authentication.
    CryptoPP::SecByteBlock sign(CryptoPP::SecByteBlock data, CryptoPP::SecByteBlock key);

    //Verifies the authenticity of ciphertext.
    //AesEncryptionError when the MAC is invalid.
    void verify(CryptoPP::SecByteBlock data, CryptoPP::SecByteBlock mac, CryptoPP::SecByteBlock key);

    //Checks if encrypted data have the minimum expected size.
    //AesEncryptionError when the size is invalid.
    void checkSize(unsigned int dataLen);
};


/// \brief AesEncryptionError exception.
class AesEncryptionError : public std::exception
{
public:
    explicit AesEncryptionError(const std::string& message);
    virtual ~AesEncryptionError() throw () {}
    virtual const char* what() const throw ();
protected:
    std::string msg;
};


//Holds SymmetricCipher objects and creates the required cipher.
class Ciphers {
public:
    //SymmetricCipher object (base class).
    typedef std::shared_ptr<CryptoPP::SymmetricCipher> Cipher;
    // Creates a new SymmetricCipher object.
    //AesEncryptionError if the cipher is not available.
    static Ciphers::Cipher getCipher(unsigned int, unsigned int);
    //Sets the key and IV (and feedback in CFB) parameters.
    static void setValues(
        Ciphers::Cipher, unsigned int, CryptoPP::SecByteBlock, CryptoPP::SecByteBlock
    );
private:
    static std::map<unsigned int, Ciphers::Cipher> ciphers;
};


//Base64 encodes / decodes data.
class Base64
{
public:
    //Encodes data.
    static CryptoPP::SecByteBlock encode(const unsigned char*, size_t);
    //Decodes data.
    static CryptoPP::SecByteBlock decode(const unsigned char*, size_t, bool);
private:
    // Checks if encoded data have the expected format.
    static void checkEncoded(const unsigned char*, size_t);
};