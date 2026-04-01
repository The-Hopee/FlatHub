#include "../include/server.hpp"
#include "../include/logger.hpp"

void Server::do_accept()
{
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if( !ec )
            {
                Logger::Instance().info("SERVER_ACCEPT", "Клиент подключился");
                std::make_shared<Session>(std::move(socket), m_factory)->start();
            }
            else
            {
                Logger::Instance().error("SERVER_ACCEPT", "Ошибка accept: " + ec.message());
            }

            do_accept(); // продолжаем принимать подключения клиентов
        }
    );
}