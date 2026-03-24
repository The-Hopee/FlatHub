#include "../../include/server.hpp"

void Server::do_accept()
{
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if( !ec )
            {
                // лог: клиент подсоединился
                std::make_shared<Session>(std::move(socket), m_factory)->start();
            }
            else
            {
                // лог: ошибка присоединения клиента
            }

            do_accept(); // продолжаем принимать подключения клиентов
        }
    );
}