#pragma once

#include <string>
#include <map>
#include <ctime>
#include <optional>

/**
 * Утилиты для работы с JWT токенами с поддержкой ролей
 * JWT структура: header.payload.signature
 * Используется для аутентификации и авторизации на основе токенов
 */
class JWTUtils {
public:
    /**
     * Структура полезной нагрузки токена
     * Содержит информацию о пользователе и время истечения токена
     */
    struct TokenPayload {
        int user_id;        // ID пользователя
        std::string login;  // Логин пользователя
        std::string role;   // Роль пользователя (user, moderator)
        time_t exp;         // Время истечения токена (UNIX timestamp)
    };

    /**
     * Генерирует JWT токен с информацией пользователя
     * 
     * Процесс создания токена:
     * 1. Создание header в JSON формате с алгоритмом (HS256) и типом (JWT)
     * 2. Создание payload с user_id, login, role и временем истечения
     * 3. Кодирование header и payload в Base64 URL формат
     * 4. Подпись сообщения HMAC-SHA256
     * 5. Объединение всех трёх частей точками
     * 
     * @param user_id ID пользователя
     * @param login Логин пользователя
     * @param role Роль пользователя (user, moderator)
     * @param expiry_seconds Время жизни токена в секундах (по умолчанию: 24 часа = 86400 сек)
     * @return Строка с JWT токеном формата "header.payload.signature"
     */
    static std::string generateToken(int user_id, const std::string& login,
                                     const std::string& role,
                                     int expiry_seconds = 86400);

    /**
     * Валидирует и декодирует JWT токен
     * 
     * Процесс валидации:
     * 1. Разбор токена на три части (header, payload, signature)
     * 2. Проверка корректности формата
     * 3. Верификация подписи HMAC-SHA256
     * 4. Декодирование payload из Base64 URL формата
     * 5. Проверка времени истечения
     * 6. Извлечение данных пользователя из payload
     * 
     * @param token JWT токен для проверки
     * @return TokenPayload если токен валиден и не истёк, std::nullopt если ошибка или истёк
     */
    static std::optional<TokenPayload> validateToken(const std::string& token);

    /**
     * Проверяет, истёк ли токен
     * @param token JWT токен для проверки
     * @return true если токен истёк, false если ещё действителен
     */
    static bool isTokenExpired(const std::string& token);

private:
    // Секретный ключ для подписи HMAC-SHA256
    // В продакшене должен быть защищён и храниться в переменных окружения
    static const std::string SECRET_KEY;

    /**
     * Кодирование в Base64 URL формат (RFC 4648)
     * 
     * Процесс кодирования:
     * 1. Использование OpenSSL BIO для стандартного Base64 кодирования
     * 2. Замена '+' на '-' и '/' на '_' для URL-безопасности
     * 3. Удаление padding символов '=' в конце строки
     * 
     * @param input Данные для кодирования
     * @return Закодированная строка в Base64 URL формате
     */
    static std::string base64_url_encode(const std::string& input);

    /**
     * Декодирование из Base64 URL формата (RFC 4648)
     * 
     * Процесс декодирования:
     * 1. Восстановление стандартных Base64 символов ('-' → '+', '_' → '/')
     * 2. Добавление padding символов '=' если нужно
     * 3. Использование OpenSSL BIO для декодирования
     * 
     * @param input Закодированная строка в Base64 URL формате
     * @return Исходные данные или пустая строка при ошибке
     */
    static std::string base64_url_decode(const std::string& input);

    /**
     * Создание подписи HMAC-SHA256
     * 
     * Процесс подписания:
     * 1. Использование функции HMAC с алгоритмом SHA256
     * 2. Подпись сообщения (header.payload) с помощью SECRET_KEY
     * 3. Кодирование полученного хеша в Base64 URL формат
     * 
     * @param message Сообщение для подписания (header.payload)
     * @return Base64 URL закодированная подпись
     */
    static std::string createSignature(const std::string& message);

    /**
     * Проверка подписи HMAC-SHA256
     * 
     * Процесс проверки:
     * 1. Создание подписи заново для переданного сообщения
     * 2. Сравнение созданной подписи с полученной подписью
     * 3. Если подписи совпадают - сообщение не было изменено
     * 
     * @param message Исходное сообщение
     * @param signature Подпись для проверки
     * @return true если подпись валидна, false если не совпадает
     */
    static bool verifySignature(const std::string& message, const std::string& signature);
};
