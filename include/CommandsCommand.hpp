#pragma once

#include "command.hpp"
#include "session.hpp"

class CreateCommandsCommand: public ICommand
{
private:
    std::shared_ptr<Session> session_;
public:
    CreateCommandsCommand( std::shared_ptr<Session> session ): session_(session) {}

    void execute() override;
};