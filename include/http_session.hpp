#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class CommandFactory;

/**
 * Обработчик одного HTTP соединения
 * 
 * Каждая HttpSession управляет соединением с одним клиентом.
 * Она отвечает за:
 * - Чтение HTTP запросов из сокета
 * - Маршрутизацию запросов к правильным обработчикам (handlers)
 * - Валидацию JWT токенов в Authorization заголовках
 * - Отправку HTTP ответов обратно клиенту
 * 
 * Жизненный цикл:
 * 1. HttpServer создаёт HttpSession при подключении клиента
 * 2. start() вызывает do_read() для ожидания первого запроса
 * 3. При получении запроса вызывается process_request()
 * 4. Отправляется ответ через do_write()
 * 5. Цикл повторяется для следующего запроса
 */
class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    /**
     * Создание новой HTTP сессии
     * 
     * @param socket TCP сокет для соединения с клиентом
     * @param factory Фабрика команд для обработки бизнес-логики
     */
    HttpSession(tcp::socket socket, std::shared_ptr<CommandFactory> factory);

    /**
     * Начало обработки запросов на этой сессии
     * 
     * Запускает асинхронное ожидание HTTP запросов от клиента.
     */
    void start();

private:
    tcp::socket socket_;                  // TCP сокет для общения с клиентом
    beast::flat_buffer buffer_;          // Буфер для чтения данных (оптимизирован для памяти)
    std::shared_ptr<CommandFactory> factory_;  // Фабрика для создания команд
    http::request<http::string_body> request_;  // Текущий HTTP запрос

    /**
     * Асинхронное чтение HTTP запроса из сокета
     * 
     * После получения полного запроса вызывает process_request()
     */
    void do_read();

    /**
     * Обработка полученного HTTP запроса
     * 
     * Парсит URL и метод, маршрутизирует к правильному handler,
     * проверяет авторизацию и отправляет ответ
     */
    void process_request();

    /**
     * Асинхронная отправка HTTP ответа клиенту
     * 
     * @param response HTTP ответ для отправки
     */
    void do_write(http::response<http::string_body> response);

    // === Обработчики REST API запросов ===
    // Каждый обработчик парсит тело запроса, обрабатывает данные
    // и возвращает JSON ответ
    
    /**
     * Обработка POST /api/login
     * Вход: { login, password }
     * Выход: { token, role, message }
     */
    http::response<http::string_body> handle_login_request(
        const http::request<http::string_body>& req);

    /**
     * Обработка POST /api/flat
     * Требует: JWT токен в Authorization заголовке
     * Вход: { house_id, flat_number, rooms, price }
     * Выход: { message, house_id }
     */
    http::response<http::string_body> handle_create_flat_request(
        const http::request<http::string_body>& req);

    /**
     * Обработка POST /api/house
     * Требует: JWT токен с ролью moderator
     * Вход: { address, build_year, developer }
     * Выход: { message, address }
     */
    http::response<http::string_body> handle_create_house_request(
        const http::request<http::string_body>& req);

    /**
     * Обработка GET /api/flats/{house_id}
     * Вход: house_id из URL пути
     * Выход: { house_id, flats[] }
     */
    http::response<http::string_body> handle_get_flats_request(
        const http::request<http::string_body>& req);

    /**
     * Обработка POST /api/flat/take
     * Требует: JWT токен с ролью moderator
     * Вход: { flat_id }
     * Выход: { message, flat_id }
     */
    http::response<http::string_body> handle_take_flat_request(
        const http::request<http::string_body>& req);

    /**
     * Обработка PUT /api/flat/status
     * Требует: JWT токен с ролью moderator
     * Вход: { flat_id, status }
     * Выход: { message, flat_id, status }
     */
    http::response<http::string_body> handle_update_flat_status_request(
        const http::request<http::string_body>& req);

    // === Утилиты для работы с HTTP ===
    
    /**
     * Создание JSON ответа с указанным статусом
     * 
     * @param status HTTP статус (200, 201, 400, 401 и т.д.)
     * @param json_body JSON строка для тела ответа
     * @return HTTP ответ с правильными заголовками
     */
    http::response<http::string_body> create_json_response(
        http::status status, const std::string& json_body);

    /**
     * Создание ошибочного JSON ответа
     * 
     * @param status HTTP статус ошибки
     * @param error_message Текст ошибки
     * @return HTTP ответ с { error: error_message }
     */
    http::response<http::string_body> create_error_response(
        http::status status, const std::string& error_message);

    /**
     * Извлечение JWT токена из Authorization заголовка
     * 
     * Ищет заголовок \"Authorization: Bearer {token}\"
     * и извлекает токен из него.
     * 
     * @param req HTTP запрос
     * @return JWT токен или пустая строка если не найден
     */
    std::string extract_bearer_token(const http::request<http::string_body>& req);

    /**
     * Валидация JWT токена и извлечение роли пользователя
     * 
     * @param token JWT токен для проверки
     * @param out_role Выходная переменная с ролью пользователя
     * @return true если токен валиден, false если ошибка или истёк
     */
    bool validate_token(const std::string& token, std::string& out_role);
};
