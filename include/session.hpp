#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <vector>
#include "factory.hpp"

using boost::asio::ip::tcp;

class Session: public std::enable_shared_from_this<Session>
{
public:
    explicit Session(tcp::socket socket, std::shared_ptr<CommandFactory> factory): m_socket(std::move(socket)), factory(factory),
    is_autorized(false), id(-1), current_login(""), current_role("") {}

    void do_read();

    void do_write( const std::string& );

    void start()
    {
        do_read();
    }

    void autorize( bool, size_t, const std::string& , const std::string& );

    void logout();

    // getter`s

    bool getStatusAutorizate() const { return is_autorized; };
    size_t getId() const { return id; }
    const std::string getCurrentLogin() const { return current_login; }
    const std::string getCurrentRole() const { return current_role; }

private:
    tcp::socket m_socket;
    std::string responce;

    std::shared_ptr<CommandFactory> factory;

    void execute_command( const std::string& );

    std::vector<std::string> parse( const std::string& );

    bool is_autorized;
    size_t id;
    std::string current_login;
    std::string current_role;

    void close();
};

#endif