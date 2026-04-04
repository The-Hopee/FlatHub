#pragma once

#include "command.hpp"
#include "FlatRepository.hpp"
#include "session.hpp"

class CreateUpdateFlatStatusCommand: public ICommand
{
private:
    size_t id_flat;
    std::string status;

    std::string token;

    std::shared_ptr<PostgresFlatRepository> repo_;

    std::shared_ptr<Session> session_;
public:
    CreateUpdateFlatStatusCommand(const std::vector<std::string>& args,
                        std::shared_ptr<PostgresFlatRepository> repo,
                        std::shared_ptr<Session> session );

    void execute() override;
};