#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include "session.hpp"
#include "factory.hpp"

using boost::asio::ip::tcp;

class Server
{
public:
    Server(boost::asio::io_context &io_context, short port, std::shared_ptr<CommandFactory> factory):
     m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)), 
     m_factory(factory)
    {
        do_accept();
    }

private:

    void do_accept();

    std::shared_ptr<CommandFactory> m_factory; 

    tcp::acceptor m_acceptor;
};

#endif