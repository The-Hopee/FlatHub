#include "../../include/client.hpp"

void Client::do_write()
{
    std::string command;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // лог: со стороны клиента нужно ввести команду
        std::getline(std::cin, command);

        if( command.empty() ) continue;

        boost::system::error_code ec;
        boost::asio::write( m_socket, boost::asio::buffer(command.append("\n")), ec);

        if( ec )
        {
            // лог: со стороны клиента ошибка в записи данных
            break;
        }
            
        if( command.compare("/quit") == 0 )
        {
            // лог: со стороны клиента инициализирован выход из сервиса
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
                // лог: со стороны клиента ошибка чтения данных
                break;
            }

            std::cout.write(buffer,length);

            std::cout << std::flush;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            break;
        }
            
    }
}