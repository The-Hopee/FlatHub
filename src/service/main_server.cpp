#include <iostream>
#include <memory>
#include <boost/asio.hpp>

#include "../include/server.hpp"
#include "../include/factory.hpp"
#include "../include/FlatRepository.hpp"
#include "../include/logger.hpp"

int main() 
{
    try 
    {
        Logger::Instance().info("MAIN_TRY", "Запуск сервера...\n");

        // 1) подключаемся к бд
        std::string db_conn = "dbname=postgres user=postgres password=secret host=127.0.0.1 port=5432";
        auto flat_repo = std::make_shared<PostgresFlatRepository>(db_conn);

        // 2) создаем нашу фабрику команд и отдаем ей базу данных
        auto command_factory = std::make_shared<CommandFactory>(flat_repo);

        boost::asio::io_context io_context;

        Server server(io_context, 8080, command_factory);

        Logger::Instance().info("MAIN_TRY", "Сервер запущен на порту 8080. Ждем клиентов...\n");

        io_context.run(); 

    } 
    catch (const std::exception& e) 
    {
        Logger::Instance().error("MAIN_FATAL_CATCH", std::string(e.what()) + "\n");
    }

    return 0;
}