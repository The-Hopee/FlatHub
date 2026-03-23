#pragma once

#include "command.hpp"

class CreateLoginCommand: public ICommand
{
private:
    std::string login, password;
public:
    CreateLoginCommand( const std::vector<std::string>& );

    void execute() override;
};