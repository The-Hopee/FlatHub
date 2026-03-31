#pragma once
#include <memory>
#include <string>
#include <optional>

struct UserData
{
    std::string login;
    std::string password;
    std::string role;
    size_t id;

    UserData(): login(""), password(""), role(""), id(-1) {}
};

class PostgresUserRepository
{
private:
    std::string conn_str_;
public:
    PostgresUserRepository( const std::string& );
    ~PostgresUserRepository() = default;
    void saveUser( const std::string&, const std::string&, const std::string& );
    std::optional<UserData> findUserByLogin( const std::string& );
};