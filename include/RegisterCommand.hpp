#pragma once

#include "command.hpp"
#include "UserRepository.hpp"

class CreateRegisterCommand: public ICommand
{
private:
    std::string login, password, role;

    std::shared_ptr<PostgresUserRepository> repo_;
public:
    CreateRegisterCommand( const std::vector<std::string>&, std::shared_ptr<PostgresUserRepository> repo);

    void execute() override;
};