#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <thread>
#include <iostream>

using boost::asio::ip::tcp;

class Client
{
public:
    Client(boost::asio::io_context& io_context, const std::string& host, short port): m_socket(io_context)
    {
        tcp::resolver resolver(io_context);
        auto endpoint = resolver.resolve(host,std::to_string(port));

        // Синхронное подключение ( нужно будет переделать на асинхронное todo )
        boost::asio::connect(m_socket,endpoint);

        std::thread(&Client::do_read, this).detach();

        do_write();
    }
private:
    tcp::socket m_socket;

    void do_read();
    
    void do_write();
};

#endif