#pragma once

#include "command.hpp"
#include "HouseRepository.hpp"

class CreateHouseCommand : public ICommand
{
private:
    std::string adress, date_of_build, builder;

    std::shared_ptr<PostgresHouseRepository> repo_;
public:
    CreateHouseCommand( const std::vector<std::string>& args, std::shared_ptr<PostgresHouseRepository> repo );

    void execute() override;
};