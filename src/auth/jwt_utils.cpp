#include "../../include/auth/jwt_utils.hpp"
#include "../../include/logger.hpp"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <sstream>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const std::string JWTUtils::SECRET_KEY = "TOTAL_CRYPTO_KEY_SHOULD_BE_SECURE_AND_HIDDEN";

// Base64 URL кодирование - преобразует данные для безопасного использования в URL
std::string JWTUtils::base64_url_encode(const std::string& input) {
    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &buffer_ptr);

    std::string result(buffer_ptr->data, buffer_ptr->length);
    BIO_free_all(bio);

    for (auto& c : result) {
        if (c == '+') c = '-';
        else if (c == '/') c = '_';
    }
    result.erase(result.find_last_not_of("=") + 1);

    return result;
}

// Base64 URL декодирование - восстанавливает исходные данные
std::string JWTUtils::base64_url_decode(const std::string& input) {
    std::string decoded = input;

    for (auto& c : decoded) {
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
    }

    while (decoded.length() % 4 != 0) {
        decoded += '=';
    }

    BIO *bio, *b64;
    char buffer[512];

    bio = BIO_new_mem_buf(decoded.data(), decoded.length());
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    int len = BIO_read(bio, buffer, sizeof(buffer) - 1);
    BIO_free_all(bio);

    if (len <= 0) return "";
    return std::string(buffer, len);
}

// HMAC-SHA256 подпись - создание криптографической подписи сообщения
std::string JWTUtils::createSignature(const std::string& message) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    HMAC(EVP_sha256(),
         SECRET_KEY.c_str(), SECRET_KEY.length(),
         (unsigned char*)message.c_str(), message.length(),
         hash, &hash_len);

    return base64_url_encode(std::string((char*)hash, hash_len));
}

// Проверка подписи - верификация целостности и подлинности
bool JWTUtils::verifySignature(const std::string& message, const std::string& signature) {
    std::string expected_signature = createSignature(message);
    return expected_signature == signature;
}

// Генерирование JWT токена
std::string JWTUtils::generateToken(int user_id, const std::string& login,
                                    const std::string& role, int expiry_seconds) {
    try {
        json header;
        header["alg"] = "HS256";
        header["typ"] = "JWT";

        auto now = std::time(nullptr);
        json payload;
        payload["user_id"] = user_id;
        payload["login"] = login;
        payload["role"] = role;
        payload["iat"] = now;
        payload["exp"] = now + expiry_seconds;

        std::string header_encoded = base64_url_encode(header.dump());
        std::string payload_encoded = base64_url_encode(payload.dump());
        std::string message = header_encoded + "." + payload_encoded;

        std::string signature = createSignature(message);
        std::string token = message + "." + signature;

        Logger::Instance().info("JWT_GENERATE", 
            "Token created for: " + login + " (role: " + role + ")");
        return token;
    } catch (const std::exception& e) {
        Logger::Instance().error("JWT_GENERATE", 
            "Error generating token: " + std::string(e.what()));
        return "";
    }
}

// Валидация JWT токена - проверка подписи, истечения и извлечение данных
std::optional<JWTUtils::TokenPayload> JWTUtils::validateToken(const std::string& token) {
    try {
        size_t first_dot = token.find('.');
        size_t second_dot = token.find('.', first_dot + 1);

        if (first_dot == std::string::npos || second_dot == std::string::npos) {
            Logger::Instance().error("JWT_VALIDATE", "Invalid token format");
            return std::nullopt;
        }

        std::string header_b64 = token.substr(0, first_dot);
        std::string payload_b64 = token.substr(first_dot + 1, second_dot - first_dot - 1);
        std::string signature = token.substr(second_dot + 1);

        std::string message = header_b64 + "." + payload_b64;
        if (!verifySignature(message, signature)) {
            Logger::Instance().error("JWT_VALIDATE", "Invalid token signature");
            return std::nullopt;
        }

        std::string payload_json = base64_url_decode(payload_b64);
        if (payload_json.empty()) {
            Logger::Instance().error("JWT_VALIDATE", "Failed to decode payload");
            return std::nullopt;
        }

        json payload = json::parse(payload_json);

        auto now = std::time(nullptr);
        time_t exp = payload["exp"].get<time_t>();
        if (now > exp) {
            Logger::Instance().info("JWT_VALIDATE", "Token expired");
            return std::nullopt;
        }

        TokenPayload result;
        result.user_id = payload["user_id"].get<int>();
        result.login = payload["login"].get<std::string>();
        result.role = payload["role"].get<std::string>();
        result.exp = exp;

        Logger::Instance().info("JWT_VALIDATE", 
            "Token validated: " + result.login + " (role: " + result.role + ")");
        return result;

    } catch (const std::exception& e) {
        Logger::Instance().error("JWT_VALIDATE", 
            "Error validating token: " + std::string(e.what()));
        return std::nullopt;
    }
}

// Проверка истечения токена
bool JWTUtils::isTokenExpired(const std::string& token) {
    auto payload = validateToken(token);
    return !payload.has_value();
}
