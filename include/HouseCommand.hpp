#pragma once

#include "command.hpp"
#include "HouseRepository.hpp"
#include <session.hpp>

class CreateHouseCommand : public ICommand
{
private:
    std::string adress, date_of_build, builder;

    std::shared_ptr<PostgresHouseRepository> repo_;

    std::shared_ptr<Session> session_;
public:
    CreateHouseCommand( const std::vector<std::string>& args,
                        std::shared_ptr<PostgresHouseRepository> repo,
                        std::shared_ptr<Session> session );

    void execute() override;
};