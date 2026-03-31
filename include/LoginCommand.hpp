#pragma once

#include "command.hpp"
#include "UserRepository.hpp"

class CreateLoginCommand: public ICommand
{
private:
    std::string login, password;

    std::shared_ptr<PostgresUserRepository> repo_;
public:
    CreateLoginCommand( const std::vector<std::string>&, std::shared_ptr<PostgresUserRepository> repo );

    void execute() override;
};