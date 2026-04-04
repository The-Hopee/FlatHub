#pragma once

#include "command.hpp"
#include "session.hpp"
#include "FlatRepository.hpp"

class CreateGetFlatsCommand: public ICommand
{
private:
    size_t house_id;

    std::string token;

    std::shared_ptr<Session> session_;

    std::shared_ptr<PostgresFlatRepository> repo_;

public:
    CreateGetFlatsCommand( const std::vector<std::string>& args, 
                            std::shared_ptr<PostgresFlatRepository> repo,
                            std::shared_ptr<Session> session);

    void execute() override;
};