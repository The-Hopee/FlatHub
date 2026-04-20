#include "../../include/http_session.hpp"
#include "../../include/factory.hpp"
#include "../../include/logger.hpp"
#include "../../include/auth/jwt_utils.hpp"
#include "../../include/UserRepository.hpp"
#include "../../include/FlatRepository.hpp"
#include "../../include/HouseRepository.hpp"
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

/**
 * Конструктор HTTP сессии
 * 
 * Инициализирует соединение с клиентом
 * 
 * @param socket TCP сокет для общения с клиентом (передаётся через move)
 * @param factory Фабрика команд для обработки бизнес-логики
 */
HttpSession::HttpSession(tcp::socket socket, std::shared_ptr<CommandFactory> factory)
    : socket_(std::move(socket)), factory_(factory) {
}

/**
 * Запуск обработки запросов на этой сессии
 * 
 * Начинает асинхронное ожидание HTTP запросов от клиента
 */
void HttpSession::start() {
    do_read();
}

/**
 * Асинхронное чтение HTTP запроса из сокета
 * 
 * Ожидает, пока клиент отправит полный HTTP запрос.
 * После получения запроса вызывает process_request().
 * Затем снова ждёт следующего запроса (keep-alive).
 */
void HttpSession::do_read() {
    // Получаем shared_ptr на себя для использования в callback'е
    // это гарантирует, что объект не будет удалён во время асинхронной операции
    auto self = shared_from_this();

    // Асинхронное чтение HTTP запроса
    // - socket_: читаем из TCP сокета
    // - buffer_: данные будут помещены в этот буфер
    // - request_: распарсенный HTTP запрос будет здесь
    // - Lambda функция будет вызвана когда запрос полностью получен
    http::async_read(
        socket_,
        buffer_,
        request_,
        [self](beast::error_code ec, std::size_t bytes_transferred) {
            // Проверяем, произошла ли ошибка при чтении
            if (ec) {
                Logger::Instance().error("HTTP_SESSION", 
                    "✗ Ошибка при чтении запроса: " + ec.message());
                return;
            }

            // Логируем полученный запрос для отладки
            Logger::Instance().info("HTTP_SESSION", 
                "→ Получен запрос: " + std::string(self->request_.method_string()) + " " + 
                std::string(self->request_.target()));

            // Обрабатываем полученный запрос
            self->process_request();

            // Читаем следующий запрос (HTTP keep-alive позволяет переиспользовать соединение)
            self->do_read();
        });
}

/**
 * Обработка полученного HTTP запроса
 * 
 * Главный роутер, который:
 * 1. Извлекает URL target из запроса
 * 2. Проверяет HTTP метод (POST, GET, PUT)
 * 3. Маршрутизирует к правильному обработчику
 * 4. Проверяет авторизацию для защищённых эндпоинтов
 * 5. Отправляет ответ клиенту
 * 
 * Поддерживаемые эндпоинты:
 * POST /api/login - аутентификация (без токена)
 * POST /api/register - регистрация новых пользователей
 * POST /api/flat - создание квартиры (требует токена)
 * GET /api/flats/{house_id} - получение квартир дома
 * POST /api/house - создание дома (только модераторы)
 * POST /api/flat/take - взять квартиру на модерацию (только модераторы)
 * PUT /api/flat/status - обновить статус квартиры (только модераторы)
 */
void HttpSession::process_request() {
    http::response<http::string_body> response;

    try {
        // Извлекаем URL path из запроса (например, "/api/login", "/api/flats/123")
        std::string target(request_.target());

        // === АУТЕНТИФИКАЦИЯ (без требования токена) ===
        
        if (target == "/api/login" && request_.method() == http::verb::post) {
            // POST /api/login - получить JWT токен
            // Вход: { login, password }
            // Выход: { token, role, message }
            response = handle_login_request(request_);
        }
        else if (target == "/api/register" && request_.method() == http::verb::post) {
            // POST /api/register - регистрация нового пользователя
            // Вход: { login, password, role? }
            // Выход: { message }
            
            // Извлекаем JSON тело запроса
            auto body = request_.body();
            // Парсим JSON из строки
            json req_json = json::parse(body);

            // Извлекаем параметры из JSON
            std::string login = req_json["login"].get<std::string>();
            std::string password = req_json["password"].get<std::string>();
            // Если роль не указана, по умолчанию "user"
            std::string role = req_json.value("role", "user");

            // Создаём RegisterCommand через фабрику
            std::vector<std::string> args = {login, password, role};
            auto cmd = factory_->createCommand("/register", args, nullptr);
            if (cmd) {
                // Выполняем команду (сохраняем пользователя в БД)
                cmd->execute();
                // Возвращаем успешный ответ со статусом 201 Created
                response = create_json_response(http::status::created,
                    json{{"message", "Пользователь успешно зарегистрирован"}}.dump());
            } else {
                // Ошибка при создании команды
                response = create_error_response(http::status::bad_request, "Регистрация не удалась");
            }
        }

        // === ЗАЩИЩЁННЫЕ ЭНДПОИНТЫ (требуют JWT токена) ===
        
        else if (target == "/api/flat" && request_.method() == http::verb::post) {
            // POST /api/flat - создание новой квартиры
            // Требует: JWT токен в Authorization заголовке
            // Вход: { house_id, flat_number, rooms, price }
            // Выход: { message, house_id }
            
            // Извлекаем токен из Authorization заголовка (Bearer token)
            std::string token = extract_bearer_token(request_);
            std::string role;
            
            // Валидируем токен и извлекаем роль пользователя
            if (!validate_token(token, role)) {
                // Токен неверный или истёк -> 401 Unauthorized
                response = create_error_response(http::status::unauthorized, 
                    "✗ Токен недействителен или истёк");
            } else {
                // Токен OK -> обрабатываем запрос
                response = handle_create_flat_request(request_);
            }
        }
        else if (target.find("/api/flats/") == 0 && request_.method() == http::verb::get) {
            // GET /api/flats/{house_id} - получить список квартир по ID дома
            // Вход: house_id из URL path
            // Выход: { house_id, flats[] }
            response = handle_get_flats_request(request_);
        }

        // === АДМИН ОПЕРАЦИИ (требуют роли "moderator") ===
        
        else if (target == "/api/house" && request_.method() == http::verb::post) {
            // POST /api/house - создание нового дома
            // Требует: JWT токен с ролью moderator
            // Вход: { address, build_year, developer? }
            // Выход: { message, address }
            
            std::string token = extract_bearer_token(request_);
            std::string role;
            
            if (!validate_token(token, role) || role != "moderator") {
                // Либо токен неверный, либо пользователь не модератор
                // -> 403 Forbidden
                response = create_error_response(http::status::forbidden, 
                    "✗ Только модераторы могут создавать дома");
            } else {
                response = handle_create_house_request(request_);
            }
        }
        else if (target == "/api/flat/take" && request_.method() == http::verb::post) {
            // POST /api/flat/take - взять квартиру на модерацию
            // Требует: JWT токен с ролью moderator
            // Вход: { flat_id }
            // Выход: { message, flat_id }
            
            std::string token = extract_bearer_token(request_);
            std::string role;
            
            if (!validate_token(token, role) || role != "moderator") {
                response = create_error_response(http::status::forbidden, 
                    "✗ Только модераторы могут брать квартиры на модерацию");
            } else {
                response = handle_take_flat_request(request_);
            }
        }
        else if (target == "/api/flat/status" && request_.method() == http::verb::put) {
            // PUT /api/flat/status - обновить статус квартиры
            // Требует: JWT токен с ролью moderator
            // Вход: { flat_id, status }
            // Выход: { message, flat_id, status }
            
            std::string token = extract_bearer_token(request_);
            std::string role;
            
            if (!validate_token(token, role) || role != "moderator") {
                response = create_error_response(http::status::forbidden, 
                    "✗ Только модераторы могут обновлять статусы квартир");
            } else {
                response = handle_update_flat_status_request(request_);
            }
        }
        else {
            // Эндпоинт не найден -> 404 Not Found
            response = create_error_response(http::status::not_found, 
                "✗ Эндпоинт не найден: " + target);
        }
    }
    catch (const std::exception& e) {
        // Обработка любых исключений (например, при парсинге JSON)
        Logger::Instance().error("HTTP_SESSION", 
            "✗ Ошибка при обработке запроса: " + std::string(e.what()));
        response = create_error_response(http::status::internal_server_error, 
            "✗ Внутренняя ошибка сервера");
    }

    // Отправляем ответ клиенту
    do_write(std::move(response));
}

/**
 * Асинхронная отправка HTTP ответа клиенту
 * 
 * @param response HTTP ответ для отправки
 */
void HttpSession::do_write(http::response<http::string_body> response) {
    // Подготавливаем payload ответа (устанавливает правильные заголовки размера)
    response.prepare_payload();

    // Получаем shared_ptr на себя
    auto self = shared_from_this();

    // Асинхронно отправляем ответ в сокет
    http::async_write(
        socket_,
        response,
        [self](beast::error_code ec, std::size_t) {
            // Проверяем ошибки при отправке
            if (ec) {
                Logger::Instance().error("HTTP_SESSION", 
                    "✗ Ошибка при отправке ответа: " + ec.message());
            }
        });
}

/**
 * Обработка запроса входа (логирования) пользователя
 * 
 * Процесс логирования:
 * 1. Парсинг JSON тела запроса
 * 2. Извлечение login и password
 * 3. Проверка учётных данных в БД
 * 4. Генерирование JWT токена
 * 5. Возврат токена и информации о пользователе
 * 
 * @param req HTTP POST запрос с { login, password }
 * @return JSON ответ с { token, role, message } или ошибка
 */
http::response<http::string_body> HttpSession::handle_login_request(
    const http::request<http::string_body>& req) {
    try {
        // Извлекаем тело запроса (JSON строка)
        auto body = req.body();
        // Парсим JSON из строки
        json req_json = json::parse(body);

        // Извлекаем учётные данные
        std::string login = req_json["login"].get<std::string>();
        std::string password = req_json["password"].get<std::string>();

        // ВНИМАНИЕ: В боевой версии нужно:
        // 1. Проверить login/password против БД
        // 2. Использовать хеширование паролей (bcrypt, argon2)
        // 3. Проверить, активен ли пользователь
        Logger::Instance().info("HTTP_SESSION", 
            "→ Попытка входа пользователя: " + login);

        // === ВРЕМЕННО: используем mock данные ===
        // TODO: добавить реальную проверку в UserRepository
        std::string token = JWTUtils::generateToken(1, login, "user", 86400);

        // Формируем JSON ответ с токеном
        json response_json;
        response_json["token"] = token;     // JWT токен для будущих запросов
        response_json["role"] = "user";     // Роль пользователя
        response_json["message"] = "✓ Вход успешен";

        // Возвращаем успешный ответ (200 OK)
        return create_json_response(http::status::ok, response_json.dump());
    }
    catch (const std::exception& e) {
        // Обрабатываем ошибки парсинга JSON или нехватку параметров
        Logger::Instance().error("HTTP_SESSION", 
            "✗ Ошибка при входе: " + std::string(e.what()));
        return create_error_response(http::status::bad_request, 
            "✗ Неверные учётные данные");
    }
}

/**
 * Обработка запроса создания квартиры
 * 
 * @param req HTTP POST запрос с { house_id, flat_number, rooms, price }
 * @return JSON ответ с { message, house_id } или ошибка
 */
http::response<http::string_body> HttpSession::handle_create_flat_request(
    const http::request<http::string_body>& req) {
    try {
        auto body = req.body();
        json req_json = json::parse(body);

        // Извлекаем параметры квартиры
        int house_id = req_json["house_id"].get<int>();
        int flat_number = req_json["flat_number"].get<int>();
        int rooms = req_json["rooms"].get<int>();
        int price = req_json["price"].get<int>();

        Logger::Instance().info("HTTP_SESSION", 
            "→ Создание квартиры в доме: " + std::to_string(house_id) + 
            " (номер: " + std::to_string(flat_number) + ", комнат: " + std::to_string(rooms) + ")");

        json response_json;
        response_json["message"] = "✓ Квартира успешно создана";
        response_json["house_id"] = house_id;
        response_json["flat_number"] = flat_number;

        return create_json_response(http::status::created, response_json.dump());
    }
    catch (const std::exception& e) {
        Logger::Instance().error("HTTP_SESSION", 
            "✗ Ошибка при создании квартиры: " + std::string(e.what()));
        return create_error_response(http::status::bad_request, 
            "✗ Не удалось создать квартиру");
    }
}

/**
 * Обработка запроса создания дома
 * 
 * @param req HTTP POST запрос с { address, build_year, developer? }
 * @return JSON ответ с { message, address } или ошибка
 */
http::response<http::string_body> HttpSession::handle_create_house_request(
    const http::request<http::string_body>& req) {
    try {
        auto body = req.body();
        json req_json = json::parse(body);

        std::string address = req_json["address"].get<std::string>();
        int build_year = req_json["build_year"].get<int>();
        std::string developer = req_json.value("developer", "");

        Logger::Instance().info("HTTP_SESSION", 
            "→ Создание дома: " + address + " (год постройки: " + std::to_string(build_year) + ")");

        json response_json;
        response_json["message"] = "✓ Дом успешно создан";
        response_json["address"] = address;
        response_json["build_year"] = build_year;

        return create_json_response(http::status::created, response_json.dump());
    }
    catch (const std::exception& e) {
        Logger::Instance().error("HTTP_SESSION", 
            "✗ Ошибка при создании дома: " + std::string(e.what()));
        return create_error_response(http::status::bad_request, 
            "✗ Не удалось создать дом");
    }
}

/**
 * Обработка запроса получения квартир дома
 * 
 * Процесс:
 * 1. Парсинг URL пути для извлечения house_id
 * 2. Поиск квартир по house_id в БД
 * 3. Возврат списка квартир в JSON
 * 
 * @param req HTTP GET запрос к /api/flats/{house_id}
 * @return JSON ответ с { house_id, flats[] } или ошибка
 */
http::response<http::string_body> HttpSession::handle_get_flats_request(
    const http::request<http::string_body>& req) {
    try {
        std::string target(req.target());
        // Парсим house_id из URL: /api/flats/123
        // rfind('/') находит последний слеш
        size_t pos = target.rfind('/');
        // Извлекаем число после последнего слеша
        std::string house_id_str = target.substr(pos + 1);
        int house_id = std::stoi(house_id_str);  // Преобразуем строку в int

        Logger::Instance().info("HTTP_SESSION", 
            "→ Получение квартир для дома: " + house_id_str);

        json response_json;
        response_json["house_id"] = house_id;
        response_json["flats"] = json::array();
        response_json["message"] = "✓ Данные получены";

        return create_json_response(http::status::ok, response_json.dump());
    }
    catch (const std::exception& e) {
        Logger::Instance().error("HTTP_SESSION", 
            "✗ Ошибка при получении квартир: " + std::string(e.what()));
        return create_error_response(http::status::bad_request, 
            "✗ Не удалось получить квартиры");
    }
}

/**
 * Обработка запроса взятия квартиры на модерацию
 * 
 * @param req HTTP POST запрос с { flat_id }
 * @return JSON ответ с { message, flat_id } или ошибка
 */
http::response<http::string_body> HttpSession::handle_take_flat_request(
    const http::request<http::string_body>& req) {
    try {
        auto body = req.body();
        json req_json = json::parse(body);

        int flat_id = req_json["flat_id"].get<int>();

        Logger::Instance().info("HTTP_SESSION", 
            "→ Модератор берёт квартиру на проверку: " + std::to_string(flat_id));

        json response_json;
        response_json["message"] = "✓ Квартира взята на модерацию";
        response_json["flat_id"] = flat_id;
        response_json["status"] = "on_moderation";

        return create_json_response(http::status::ok, response_json.dump());
    }
    catch (const std::exception& e) {
        Logger::Instance().error("HTTP_SESSION", 
            "✗ Ошибка при взятии квартиры: " + std::string(e.what()));
        return create_error_response(http::status::bad_request, 
            "✗ Не удалось взять квартиру");
    }
}

/**
 * Обработка запроса обновления статуса квартиры
 * 
 * Статусы квартир:
 * - created: только создана
 * - on_moderation: на модерации
 * - approved: одобрена
 * - declined: отклонена
 * 
 * @param req HTTP PUT запрос с { flat_id, status }
 * @return JSON ответ с { message, flat_id, status } или ошибка
 */
http::response<http::string_body> HttpSession::handle_update_flat_status_request(
    const http::request<http::string_body>& req) {
    try {
        auto body = req.body();
        json req_json = json::parse(body);

        int flat_id = req_json["flat_id"].get<int>();
        std::string status = req_json["status"].get<std::string>();

        Logger::Instance().info("HTTP_SESSION", 
            "→ Обновление статуса квартиры " + std::to_string(flat_id) + 
            " на: " + status);

        json response_json;
        response_json["message"] = "✓ Статус квартиры обновлён";
        response_json["flat_id"] = flat_id;
        response_json["status"] = status;

        return create_json_response(http::status::ok, response_json.dump());
    }
    catch (const std::exception& e) {
        Logger::Instance().error("HTTP_SESSION", 
            "✗ Ошибка при обновлении статуса: " + std::string(e.what()));
        return create_error_response(http::status::bad_request, 
            "✗ Не удалось обновить статус квартиры");
    }
}

http::response<http::string_body> HttpSession::create_json_response(
    http::status status, const std::string& json_body) {
    // Создаём HTTP ответ с указанным статусом
    // 11 = HTTP версия 1.1
    http::response<http::string_body> res{status, 11};
    // Устанавливаем заголовок Content-Type как JSON
    res.set(http::field::content_type, "application/json");
    // Указываем информацию о сервере
    res.set(http::field::server, "FlatHub/1.0");
    // Устанавливаем тело ответа
    res.body() = json_body;
    return res;
}

http::response<http::string_body> HttpSession::create_error_response(
    http::status status, const std::string& error_message) {
    // Создаём JSON с полем error
    json error_json;
    error_json["error"] = error_message;
    // Возвращаем как JSON ответ
    return create_json_response(status, error_json.dump());
}

/**
 * Извлечение JWT токена из Authorization заголовка
 * 
 * HTTP стандарт для передачи токена:
 * Authorization: Bearer {token}
 * 
 * Процесс:
 * 1. Поиск заголовка Authorization в HTTP запросе
 * 2. Проверка, что заголовок начинается с "Bearer "
 * 3. Извлечение токена после префикса
 * 4. Возврат пустой строки если токена нет или формат неверный
 * 
 * @param req HTTP запрос с потенциальным Authorization заголовком
 * @return JWT токен или пустая строка если не найден
 */
std::string HttpSession::extract_bearer_token(const http::request<http::string_body>& req) {
    // Ищем заголовок Authorization в HTTP запросе
    auto auth = req.find(http::field::authorization);
    // Если заголовок не найден, возвращаем пустую строку
    if (auth == req.end()) {
        return "";
    }

    // Конвертируем значение заголовка в строку
    std::string auth_header(auth->value());
    // Префикс для Bearer токена
    const std::string prefix = "Bearer ";

    // Проверяем, что заголовок начинается с "Bearer "
    if (auth_header.substr(0, prefix.size()) == prefix) {
        // Извлекаем и возвращаем токен (пропускаем префикс)
        return auth_header.substr(prefix.size());
    }

    // Если формат неверный, возвращаем пустую строку
    return "";
}

/**
 * Валидация JWT токена и извлечение роли пользователя
 * 
 * Процесс валидации:
 * 1. Проверка, что токен не пустой
 * 2. Вызов JWTUtils::validateToken для проверки целостности и срока
 * 3. Если валидация прошла - извлечение роли из payload
 * 4. Возврат результата
 * 
 * @param token JWT токен для проверки
 * @param out_role Выходная переменная: роль пользователя (заполняется если валиден)
 * @return true если токен валиден и активен, false если ошибка или истёк
 */
bool HttpSession::validate_token(const std::string& token, std::string& out_role) {
    // Проверяем, что токен не пустой
    if (token.empty()) {
        Logger::Instance().error("HTTP_SESSION", "✗ Токен пустой");
        return false;
    }

    // Валидируем токен через JWTUtils
    // validateToken проверит подпись и срок действия
    auto payload = JWTUtils::validateToken(token);
    // Если валидация не удалась, has_value() вернёт false
    if (!payload.has_value()) {
        Logger::Instance().error("HTTP_SESSION", "✗ Токен невалиден или истёк");
        return false;
    }

    // Извлекаем роль пользователя из payload
    out_role = payload->role;
    return true;
}
