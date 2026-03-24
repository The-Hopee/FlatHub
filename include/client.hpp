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
    Client(boost::asio::io_context& io_context, const std::string& host, short port)
        : m_socket(io_context), m_host(host), m_port(port) {}

    ~Client() 
    {
        if (m_read_thread.joinable()) 
        {
            m_read_thread.join();
        }
    }

    void start() 
    {
        tcp::resolver resolver(m_socket.get_executor());
        auto endpoint = resolver.resolve(m_host, std::to_string(m_port));

        boost::asio::connect(m_socket, endpoint);

        m_read_thread = std::thread(&Client::do_read, this);

        do_write(); 
    }
private:
    tcp::socket m_socket;

    std::string m_host;
    short m_port;

    std::thread m_read_thread;

    void do_read();
    
    void do_write();
};

#endif