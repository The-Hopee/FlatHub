#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <vector>

using boost::asio::ip::tcp;

class Session: public std::enable_shared_from_this<Session>
{
public:
    explicit Session(tcp::socket socket, std::shared_ptr<CommandFactory> factory): m_socket(std::move(socket)), factory(factory) {}

    void start()
    {
        do_read();
    }
private:
    tcp::socket m_socket;
    std::string responce;

    std::shared_ptr<CommandFactory> factory;

    void do_read();

    void do_write( const std::string& );

    void execute_command( const std::string& );

    std::vector<std::string> parse( const std::string& );
};

#endif