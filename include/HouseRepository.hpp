#pragma once

#include <memory>
#include <string>

class PostgresHouseRepository
{
private:
    std::string conn_str_;
public:
    PostgresHouseRepository( const std::string& );
    ~PostgresHouseRepository() = default;
    void saveHouse( const std::string& adress, const std::string& date_of_build, const std::string& builder);
};