#include "../../include/session.hpp"
#include "../../include/factory.hpp"
#include "../../include/logger.hpp"

void Session::do_read()
{
    boost::asio::async_read_until(
        m_socket,
        boost::asio::dynamic_buffer(responce),
        "\n",
        [this, self = shared_from_this()]( boost::system::error_code ec, std::size_t length)
        {
            if( !ec )
            {
                std::string client_msg = responce.substr(0,length);
                responce.erase(0, length+1);
                execute_command(client_msg);
                self->do_read();
            }
            else
            {
                //здесь должен быть вызван логер с нужным сообщением
            }
        }
    );
}

void Session::do_write( const std::string& msg )
{
    boost::asio::async_write(
        m_socket,
        boost::asio::buffer(msg),
        [this, self = shared_from_this()]( boost::system::error_code ec, std::size_t length )
        {
            if( ec )
            {
                // сообщение логера
            }
        }
    );
}

void Session::execute_command( const std::string& line)
{
    // здесь менеджер вызывает метод своего класса, чтобы сделать запись квартиры в бд.
    std::vector<std::string> parse_line = parse(line);
    std::string command = parse_line[0];
    parse_line.erase(parse_line.begin()); // стираем команду

    try
    {
        auto command_from_factory = factory_.createCommand(command, parse_line);

        if( command_from_factory )
        {
            command_from_factory->execute();
        }
        else
        {
            Logger::Instance().error("EXECUTE_COMMAND _TRY","Ошибка: неизвестная команда!");
        }
    }
    catch(const std::exception& e)
    {
        Logger::Instance().error("EXECUTE_COMMAND_CATCH", "Поймано исключение: " + std::string(e.what()));
    }
}

std::vector<std::string> Session::parse( const std::string& line )
{
    if( line.empty() )
    {
        return {};
    }

    std::vector<std::string> commands;
    std::string temp_str;

    for( auto it: line )
    {
        if( it == ' ' )
        {
            commands.push_back(temp_str);
            temp_str.erase();
        }
        else
        {
            temp_str.push_back(it);
        }
    }

    if( !temp_str.empty() )
    {
        commands.push_back(temp_str);
    }

    return commands;
}