#include "../../include/client.hpp"
#include "../../include/logger.hpp" // <--- НЕ ЗАБУДЬ ПОДКЛЮЧИТЬ ЛОГГЕР!
#include <iostream>

void Client::do_write()
{
    std::string command;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Для пользователя лучше вывести простую стрелочку в консоль, чтобы он понимал, что можно вводить
        std::cout << "Введите команду (или /quit для выхода): ";
        std::getline(std::cin, command);

        if( command.empty() ) continue;

        boost::system::error_code ec;
        boost::asio::write( m_socket, boost::asio::buffer(command + "\n"), ec);

        if( ec )
        {
            Logger::Instance().error("CLIENT_WRITE", "Ошибка отправки данных: " + ec.message() + "\n");
            break;
        }

        if( command.compare("/quit") == 0 )
        {
            Logger::Instance().info("CLIENT_WRITE", "Инициализирован выход из сервиса. Отключение...\n");
            break;
        }
    }

    m_socket.close();
}

void Client::do_read()
{
    while(true)
    {
        try
        {
            char buffer[1024];
            boost::system::error_code ec;

            std::size_t length = m_socket.read_some(boost::asio::buffer(buffer), ec);

            if( ec )
            {
                // Если сервер просто закрыл соединение, это не страшная ошибка (EOF - End Of File)
                if (ec == boost::asio::error::eof) {
                    Logger::Instance().info("CLIENT_READ", "Сервер разорвал соединение.\n");
                } else {
                    Logger::Instance().error("CLIENT_READ", "Ошибка чтения данных: " + ec.message() + "\n");
                }
                break;
            }

            // Выводим ответ от сервера пользователю
            std::cout.write(buffer, length);
            std::cout << std::flush;
        }
        catch(const std::exception& e)
        {
            // Лог: поймали критическое исключение (например, разрыв сети на уровне ОС)
            Logger::Instance().error("CLIENT_EXCEPTION", "Критическая ошибка: " + std::string(e.what()) + "\n");
            break;
        }
    }
}