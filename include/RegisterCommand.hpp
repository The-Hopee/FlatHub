#pragma once

#include "command.hpp"
#include "UserRepository.hpp"
#include "session.hpp"

class CreateRegisterCommand: public ICommand
{
private:
    std::string login, password, role;

    std::shared_ptr<PostgresUserRepository> repo_;

    std::shared_ptr<Session> session_;
public:
    CreateRegisterCommand( const std::vector<std::string>&, std::shared_ptr<PostgresUserRepository> repo, std::shared_ptr<Session> session);

    void execute() override;
};