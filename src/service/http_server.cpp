#include "../../include/http_server.hpp"
#include "../../include/http_session.hpp"
#include "../../include/logger.hpp"
#include <boost/bind/bind.hpp>

/**
 * Конструктор HTTP сервера
 * 
 * Инициализирует сервер и привязывает его к порту.
 * Сервер готов принимать соединения после вызова start()
 * 
 * @param io_context Контекст ASIO для асинхронных операций
 * @param port Порт для прослушивания (обычно 8080)
 * @param factory Фабрика команд для обработки запросов
 */
HttpServer::HttpServer(net::io_context& io_context, unsigned short port,
                       std::shared_ptr<CommandFactory> factory)
    : io_context_(io_context),
      // Создаём acceptor, привязанный к IPv4 на указанном порту
      // tcp::v4() - используем IPv4 протокол
      // tcp::endpoint(tcp::v4(), port) - слушаем на всех интерфейсах на порту
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      factory_(factory) {
    Logger::Instance().info("HTTP_SERVER", 
        "✓ HTTP сервер инициализирован на порту " + std::to_string(port));
}

/**
 * Запуск сервера
 * 
 * Начинает асинхронное ожидание входящих соединений.
 * Каждое соединение будет обработано в отдельной HttpSession.
 */
void HttpServer::start() {
    Logger::Instance().info("HTTP_SERVER", "→ Запуск HTTP сервера...");
    // Начинаем асинхронное ожидание первого соединения
    do_accept();
}

/**
 * Остановка сервера
 * 
 * Закрывает acceptor, что прекратит приём новых соединений.
 * Существующие соединения продолжат работать.
 */
void HttpServer::stop() {
    Logger::Instance().info("HTTP_SERVER", "↓ Остановка HTTP сервера...");
    // Закрываем acceptor - он перестанет принимать новые соединения
    acceptor_.close();
}

/**
 * Запуск асинхронного ожидания входящего соединения
 * 
 * Регистрирует callback, которая будет вызвана когда:
 * - Клиент подключится (успех)
 * - Произойдёт ошибка (например, порт занят)
 */
void HttpServer::do_accept() {
    // Асинхронная функция async_accept ждёт входящего соединения
    // Когда соединение будет установлено, вызовется lambda функция on_accept
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            // this - указатель на текущий объект HttpServer
            // ec - код ошибки (0 = успех)
            // socket - новый TCP сокет для общения с клиентом
            on_accept(ec, std::move(socket));
        });
}

/**
 * Callback функция при получении нового соединения
 * 
 * Создаёт новую HttpSession для клиента и регистрирует следующее ожидание
 * 
 * @param ec Код ошибки (если есть)
 * @param socket TCP сокет для нового соединения
 */
void HttpServer::on_accept(beast::error_code ec, tcp::socket socket) {
    // Проверяем, произошла ли ошибка при принятии соединения
    if (ec) {
        Logger::Instance().error("HTTP_SERVER", 
            "✗ Ошибка при принятии соединения: " + ec.message());
        return;
    }

    Logger::Instance().info("HTTP_SERVER", "✓ Новый клиент подключился");

    // === Создание новой HttpSession для клиента ===
    // std::make_shared создаёт общий указатель (shared_ptr) на новый объект
    // std::move(socket) передаёт сокет HttpSession (перемещение, а не копирование)
    // factory_ - фабрика команд для обработки HTTP запросов
    auto session = std::make_shared<HttpSession>(std::move(socket), factory_);
    // Запускаем сессию: она начнёт ожидать HTTP запросов от клиента
    session->start();

    // === Продолжаем ожидание следующего клиента ===
    // Регистрируем ещё одно async_accept для следующего соединения
    // Это позволяет серверу одновременно обслуживать множество клиентов
    do_accept();
}
