#pragma once

#include "command.hpp"
#include "UserRepository.hpp"
#include "session.hpp"

class CreateLoginCommand: public ICommand
{
private:
    std::string login, password;

    std::shared_ptr<PostgresUserRepository> repo_;

    std::shared_ptr<Session> session_;
public:
    CreateLoginCommand( const std::vector<std::string>&, std::shared_ptr<PostgresUserRepository> repo,  std::shared_ptr<Session> session);

    void execute() override;
};