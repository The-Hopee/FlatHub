#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class HttpSession;
class CommandFactory;

/**
 * HTTP сервер для обработки REST API запросов
 * Поддерживает SSL/TLS для защищённого соединения
 * 
 * Основные функции:
 * - Слушание входящих TCP соединений на указанном порте
 * - Создание новой HttpSession для каждого клиента
 * - Асинхронная обработка запросов через Boost.Asio
 * - Интеграция с CommandFactory для обработки бизнес-логики
 */
class HttpServer {
public:
    /**
     * Создание HTTP сервера
     * 
     * @param io_context ASIO io_context (управляет асинхронными операциями)
     * @param port Порт для прослушивания входящих соединений (обычно 8080 или 80)
     * @param factory Фабрика команд для обработки REST запросов
     */
    HttpServer(net::io_context& io_context, unsigned short port,
               std::shared_ptr<CommandFactory> factory);

    /**
     * Начало прослушивания входящих соединений
     * 
     * Запускает асинхронное ожидание подключения клиентов.
     * При подключении клиента создаётся новая HttpSession.
     */
    void start();

    /**
     * Остановка сервера
     * 
     * Закрывает acceptor, так что новые соединения больше не будут приниматься.
     * Текущие соединения не закрываются.
     */
    void stop();

private:
    // Контекст ввода-вывода Boost.Asio (управляет асинхронными операциями)
    net::io_context& io_context_;
    // Acceptor - объект для прослушивания входящих соединений
    tcp::acceptor acceptor_;
    // Фабрика команд для создания объектов команд при обработке запросов
    std::shared_ptr<CommandFactory> factory_;

    /**
     * Запуск асинхронного ожидания входящего соединения
     * 
     * Регистрирует callback функцию, которая будет вызвана
     * когда клиент подключится.
     */
    void do_accept();

    /**
     * Callback функция при получении нового соединения
     * 
     * @param ec Код ошибки (если есть)
     * @param socket TCP сокет для соединения с клиентом
     */
    void on_accept(beast::error_code ec, tcp::socket socket);
};
