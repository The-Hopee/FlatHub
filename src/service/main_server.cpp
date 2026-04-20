#include <iostream>
#include <memory>
#include <boost/asio.hpp>

#include "../include/http_server.hpp"
#include "../include/factory.hpp"
#include "../include/DatabaseManager.hpp"
#include "../include/logger.hpp"

int main() 
{
    try 
    {
        Logger::Instance().info("MAIN_TRY", "Starting HTTP server with JWT authentication...");

        // 1) Connect to database
        std::string db_conn = "dbname=flat_hub_db user=postgres password=secret host=127.0.0.1 port=5432";
        auto databaseManager = std::make_shared<DatabaseManager>(db_conn);

        // 2) Create command factory
        auto command_factory = std::make_shared<CommandFactory>(databaseManager);

        boost::asio::io_context io_context;

        // 3) Start HTTP server
        HttpServer http_server(io_context, 8080, command_factory);
        http_server.start();

        Logger::Instance().info("MAIN_TRY", "HTTP Server started on port 8080. Listening for requests...");
        Logger::Instance().info("MAIN_TRY", "Available endpoints: /api/login, /api/register, /api/flat, /api/flats/{id}, /api/house");

        io_context.run(); 

    } 
    catch (const std::exception& e) 
    {
        Logger::Instance().error("MAIN_FATAL_CATCH", "Fatal error: " + std::string(e.what()));
    }

    return 0;
}