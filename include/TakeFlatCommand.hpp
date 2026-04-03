#pragma once

#include "command.hpp"
#include "FlatRepository.hpp"
#include "session.hpp"

class CreateTakeFlatCommand: public ICommand
{
private:
    size_t id_flat;

    std::shared_ptr<PostgresFlatRepository> repo_;

    std::shared_ptr<Session> session_;
public:
    CreateTakeFlatCommand(const std::vector<std::string>& args,
                        std::shared_ptr<PostgresFlatRepository> repo,
                        std::shared_ptr<Session> session );

    void execute() override;
};