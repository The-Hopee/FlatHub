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

        if( command == "/quit" || command == "/commands" )
        {
            boost::asio::write( m_socket, boost::asio::buffer(command + "\n"), ec);   
        }
        else if( command.find("/login") != std::string::npos ||
            command.find("/register") != std::string::npos )
        {
            size_t pos = command.find(' ');
            if( pos != std::string::npos )
            {
                boost::asio::write( m_socket, boost::asio::buffer(command + "\n"), ec);
            }
            else
            {
                Logger::Instance().error("CLIENT_WRITE", "Указана пустая команда! Отказ.");
            }
        }
        else
        {
            size_t pos = command.find(' '); // ищем первый пробел после команды
            if( pos != std::string::npos )
            {
                command.insert(pos+1, token + " "); // вставляем наш токен после пробела
                boost::asio::write( m_socket, boost::asio::buffer(command + "\n"), ec);
            }
            else
            {
                Logger::Instance().error("CLIENT_WRITE", "Указана пустая команда! Отказ.");
            }
        }

        if( ec )
        {
            Logger::Instance().error("CLIENT_WRITE", "Ошибка отправки данных: " + ec.message());
            break;
        }

        if( command.compare("/quit") == 0 )
        {
            Logger::Instance().info("CLIENT_WRITE", "Инициализирован выход из сервиса. Отключение...");
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
                    Logger::Instance().info("CLIENT_READ", "Сервер разорвал соединение.");
                } else {
                    Logger::Instance().error("CLIENT_READ", "Ошибка чтения данных: " + ec.message());
                }
                break;
            }

            // Выводим ответ от сервера пользователю
            std::string ans{buffer, length};

            if( ans.find("TOKEN: ") != std::string::npos )
            {
                size_t pos = ans.find(' ');

                if( pos != std::string::npos )
                {
                    token = ans.substr(pos+1);

                    if (!token.empty() && token.back() == '\n') token.pop_back();

                    if (!token.empty() && token.back() == '\r') token.pop_back();
                }
            }

            std::cout << ans;
            std::cout << std::flush;
        }
        catch(const std::exception& e)
        {
            // Лог: поймали критическое исключение (например, разрыв сети на уровне ОС)
            Logger::Instance().error("CLIENT_EXCEPTION", "Критическая ошибка: " + std::string(e.what()));
            break;
        }
    }
}