#pragma once
#include "command.hpp"
#include "session.hpp"

class CreateQuitCommand: public ICommand
{
private:
    std::shared_ptr<Session> session_;
public:
    CreateQuitCommand( std::shared_ptr<Session> session );

    void execute() override;
};