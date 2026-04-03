#pragma once
#include <memory>
#include <string>
#include <vector>

struct FlatData
{
    size_t id;
    size_t house_id;
    size_t flatNumber;
    size_t price;
    size_t rooms;
    std::string status;

    FlatData(): id(-1), house_id(-1), flatNumber(-1), price(-1), rooms(-1), status("") {}
};

class PostgresFlatRepository
{
public:
    PostgresFlatRepository( const std::string& );
    ~PostgresFlatRepository() = default;
    void saveFlat(int house_id, int flat_num, int rooms, int price);

    std::vector<FlatData> getFlatForHouseID(size_t house_id, const std::string&) const;

    bool takeFlat( size_t flat_id );

    bool UpdateFlatStatus( size_t flat_id, const std::string& status );
private:
    std::string conn_str_;
};