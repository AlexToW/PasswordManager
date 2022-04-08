#include "encryption.h"

#include <iostream>
#include <regex>

#include <cryptopp/osrng.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/hmac.h>
#include <cryptopp/hkdf.h>
#include <cryptopp/base64.h>


AesEncryption::AesEncryption(std::string mode, unsigned int size)
{
    std::transform(mode.begin(), mode.end(), mode.begin(), ::toupper);

    if (this->modes.count(mode) == 0) {
        throw std::runtime_error((mode + " is not supported!").c_str());
    }
    if (std::find(std::begin(sizes), std::end(sizes), size) == std::end(sizes)) {
        throw std::runtime_error("Invalid key size!");
    }
    this->mode = mode;
    this->keyLen = size / 8;
}

CryptoPP::SecByteBlock AesEncryption::encrypt(
    CryptoPP::SecByteBlock data, CryptoPP::SecByteBlock password)
{
    try {
        CryptoPP::SecByteBlock salt = this->randomBytes(saltLen);
        CryptoPP::SecByteBlock iv = this->randomBytes(ivLen);

        CryptoPP::SecByteBlock aesKey(this->keyLen), macKey(this->macKeyLen);
        this->keys(password, salt, aesKey, macKey);

        Ciphers::Cipher cipher = this->cipher(aesKey, iv, AesEncryption::ENCRYPT);
        CryptoPP::StreamTransformationFilter stf(*cipher, NULL);
        stf.Put(data.data(), data.size());
        stf.MessageEnd();

        CryptoPP::SecByteBlock ciphertext(stf.MaxRetrievable());
        stf.Get(ciphertext, ciphertext.size());

        CryptoPP::SecByteBlock mac = this->sign(iv + ciphertext, macKey);
        CryptoPP::SecByteBlock encrypted = salt + iv + ciphertext + mac;
        if (this->base64) {
            encrypted = Base64::encode(encrypted.data(), encrypted.size());
        }
        return encrypted;
    }
    catch (const CryptoPP::Exception& e) {
        this->errorHandler(e);
    }
    catch (const AesEncryptionError& e) {
        this->errorHandler(e);
    }
    return CryptoPP::SecByteBlock(0);
}

CryptoPP::SecByteBlock AesEncryption::encrypt(std::string data, std::string password)
{
    CryptoPP::SecByteBlock _data((unsigned char*)data.data(), data.size());
    CryptoPP::SecByteBlock _password((unsigned char*)password.data(), password.size());
    return this->encrypt(_data, _password);
}

CryptoPP::SecByteBlock AesEncryption::decrypt(
    CryptoPP::SecByteBlock data, CryptoPP::SecByteBlock password)
{
    try {
        if (this->base64) {
            data = Base64::decode(data.data(), data.size(), true);
        }
        this->checkSize(data.size());

        CryptoPP::SecByteBlock salt(data.data(), saltLen);
        CryptoPP::SecByteBlock iv(data.data() + saltLen, ivLen);
        CryptoPP::SecByteBlock ciphertext(
            data.data() + saltLen + ivLen, data.size() - saltLen - ivLen - macLen
        );
        CryptoPP::SecByteBlock mac(
            data.data() + saltLen + ivLen + ciphertext.size(), macLen
        );

        CryptoPP::SecByteBlock aesKey(this->keyLen), macKey(this->macKeyLen);
        this->keys(password, salt, aesKey, macKey);
        this->verify(iv + ciphertext, mac, macKey);

        Ciphers::Cipher cipher = this->cipher(aesKey, iv, AesEncryption::DECRYPT);
        CryptoPP::StreamTransformationFilter stf(*cipher.get(), NULL);
        stf.Put(ciphertext.data(), ciphertext.size());
        stf.MessageEnd();

        CryptoPP::SecByteBlock plaintext(stf.MaxRetrievable());
        stf.Get(plaintext.data(), plaintext.size());
        return plaintext;
    }
    catch (const CryptoPP::Exception& e) {
        this->errorHandler(e);
    }
    catch (const AesEncryptionError& e) {
        this->errorHandler(e);
    }
    return CryptoPP::SecByteBlock(0);
}

CryptoPP::SecByteBlock AesEncryption::decrypt(std::string data, std::string password)
{
    CryptoPP::SecByteBlock _data((unsigned char*)data.data(), data.size());
    CryptoPP::SecByteBlock _password((unsigned char*)password.data(), password.size());
    return this->decrypt(_data, _password);
}


void AesEncryption::keys(CryptoPP::SecByteBlock password, const unsigned char* salt,
    CryptoPP::SecByteBlock& aesKey, CryptoPP::SecByteBlock& macKey)
{
    CryptoPP::SecByteBlock dkey(this->keyLen + this->macKeyLen);
    if (!password.empty()) {
        CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> kdf;
        kdf.DeriveKey(
            dkey.data(), dkey.size(), 0x00, password, password.size(),
            salt, this->saltLen, this->keyIterations
        );
    }
    else {
        throw AesEncryptionError("No password or key spacified!");
    }
    aesKey.Assign(dkey.begin(), this->keyLen);
    macKey.Assign(dkey.begin() + this->keyLen, this->macKeyLen);
}

CryptoPP::SecByteBlock AesEncryption::randomBytes(size_t size)
{
    CryptoPP::SecByteBlock rb(size);
    CryptoPP::AutoSeededRandomPool prng;

    prng.GenerateBlock(rb, size);
    return rb;
}

std::shared_ptr<CryptoPP::SymmetricCipher> AesEncryption::cipher(
    CryptoPP::SecByteBlock key, CryptoPP::SecByteBlock iv, unsigned int encMode)
{
    unsigned int aesMode = this->modes.at(this->mode);
    Ciphers::Cipher cipher = Ciphers::getCipher(aesMode, encMode);

    Ciphers::setValues(cipher, aesMode, key, iv);
    return cipher;
}

CryptoPP::SecByteBlock AesEncryption::sign(CryptoPP::SecByteBlock data, CryptoPP::SecByteBlock key)
{
    CryptoPP::HMAC<CryptoPP::SHA256> hmac(key, macKeyLen);
    CryptoPP::SecByteBlock mac(macLen);

    hmac.Update(data, data.size());
    hmac.Final(mac);
    return mac;
}

void AesEncryption::verify(
    CryptoPP::SecByteBlock data, CryptoPP::SecByteBlock mac, CryptoPP::SecByteBlock key)
{
    CryptoPP::HMAC<CryptoPP::SHA256> hmac(key, macKeyLen);

    hmac.Update(data.data(), data.size());
    if (!hmac.Verify(mac)) {
        throw AesEncryptionError("MAC check failed!");
    }
}

void AesEncryption::errorHandler(const std::exception& exception)
{
    std::cout << exception.what() << std::endl;
}

void AesEncryption::checkSize(unsigned int dataLen)
{
    unsigned int simLen = this->saltLen + this->ivLen + this->macLen;
    unsigned int ctLen = (this->mode == "CBC") ? CryptoPP::AES::BLOCKSIZE : 0;

    if (dataLen < simLen + ctLen) {
        throw AesEncryptionError("Invalid data size!");
    }
    if (this->mode == "CBC" && (dataLen - simLen) % CryptoPP::AES::BLOCKSIZE != 0) {
        throw AesEncryptionError("Invalid data size!");
    }
}

AesEncryptionError::AesEncryptionError(const std::string& message)
{
    msg = message;
}

const char* AesEncryptionError::what() const throw ()
{
    return msg.c_str();
}

Ciphers::Cipher Ciphers::getCipher(unsigned int aesMode, unsigned int encMode)
{
    if (Ciphers::ciphers.find(aesMode + encMode) == Ciphers::ciphers.end()) {
        throw AesEncryptionError("Invalid mode or method!");
    }
    Ciphers::Cipher cipher = Ciphers::ciphers.at(aesMode + encMode);
    return cipher;
}

void Ciphers::setValues(
    Ciphers::Cipher cipher, unsigned int mode, CryptoPP::SecByteBlock key, CryptoPP::SecByteBlock iv)
{
    CryptoPP::ConstByteArrayParameter IV(iv);
    CryptoPP::AlgorithmParameters params = CryptoPP::MakeParameters(CryptoPP::Name::IV(), IV);

    if (mode == AesEncryption::CFB) {
        params(CryptoPP::Name::FeedbackSize(), 1);
    }
    cipher->SetKey(key, key.size(), params);
}

std::map<unsigned int, Ciphers::Cipher> Ciphers::ciphers = {
    { 11, Ciphers::Cipher(new CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption()) },
    { 12, Ciphers::Cipher(new CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption()) },
    { 21, Ciphers::Cipher(new CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption()) },
    { 22, Ciphers::Cipher(new CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption()) }
};

CryptoPP::SecByteBlock Base64::encode(const unsigned char* data, size_t size)
{
    CryptoPP::Base64Encoder encoder(NULL, false);
    encoder.Put(data, size);
    encoder.MessageEnd();

    CryptoPP::SecByteBlock encoded(encoder.MaxRetrievable());
    encoder.Get(encoded.data(), encoded.size());
    return encoded;
}

CryptoPP::SecByteBlock Base64::decode(const unsigned char* data, size_t size, bool check)
{
    if (check) {
        Base64::checkEncoded(data, size);
    }
    CryptoPP::Base64Decoder decoder;
    decoder.Put(data, size);
    decoder.MessageEnd();

    CryptoPP::SecByteBlock decoded(decoder.MaxRetrievable());
    decoder.Get(decoded.data(), decoded.size());
    return decoded;
}

void Base64::checkEncoded(const unsigned char* data, size_t size)
{
    std::string validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=\n";

    if (size % 4 != 0) {
        throw AesEncryptionError("Invalid base64 format!");
    }
    for (size_t i = 0; i < size; i++) {
        if (validChars.find(data[i]) == std::string::npos) {
            throw AesEncryptionError("Invalid base64 format!");
        }
    }
}

