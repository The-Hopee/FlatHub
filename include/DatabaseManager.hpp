#pragma once

#include "FlatRepository.hpp"
#include "HouseRepository.hpp"
#include "UserRepository.hpp"
#include <memory>

class DatabaseManager
{
private:
    std::shared_ptr<PostgresFlatRepository>  flat_repo;
    std::shared_ptr<PostgresHouseRepository> house_repo;
    std::shared_ptr<PostgresUserRepository>  user_repo;
public:
    DatabaseManager( const std::string& conn ): 
    flat_repo(std::make_shared<PostgresFlatRepository>(conn)), 
    house_repo(std::make_shared<PostgresHouseRepository>(conn)),
    user_repo(std::make_shared<PostgresUserRepository>(conn))
    {}

    std::shared_ptr<PostgresFlatRepository>  getFlatRepo()  { return flat_repo; }
    std::shared_ptr<PostgresHouseRepository> getHouseRepo() { return house_repo; }
    std::shared_ptr<PostgresUserRepository>  getUserRepo()  { return user_repo; }
};