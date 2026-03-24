#include <iostream>
#include <thread>
#include <boost/asio.hpp>

#include "../include/client.hpp"
#include "../include/logger.hpp"

using boost::asio::ip::tcp;

int main() 
{
    try 
    {
        Logger::Instance().info("MAIN_CLIENT_TRY","--- Клиент Сервиса Домов ---\n");
        Logger::Instance().info("MAIN_CLIENT_TRY","Подключение к серверу...\n");

        boost::asio::io_context io_context;

        Client client(io_context, "127.0.0.1", 8080);

        client.start();

    } 
    catch (const std::exception& e) 
    {
        Logger::Instance().error("MAIN_CLIENT_CATCH","Критическая ошибка клиента: " + std::string(e.what()) + "\n");
    }

    return 0;
}