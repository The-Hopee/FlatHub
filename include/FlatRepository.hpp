#pragma once
#include <memory>
#include <string>

class PostgresFlatRepository
{
public:
    PostgresFlatRepository( const std::string& );
    ~PostgresFlatRepository() = default;
    void saveFlat(int house_id, int flat_num, int rooms, int price);
private:
    std::string conn_str_;
};