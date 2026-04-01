#pragma once

#include "command.hpp"
#include "FlatRepository.hpp"
#include "session.hpp"

class CreateFlatCommand : public ICommand
{
private:
    size_t house_id, flat_number, price, rooms;

    std::shared_ptr<PostgresFlatRepository> repo_;

    std::shared_ptr<Session> session_;
public:
    CreateFlatCommand( const std::vector<std::string>&,
                         std::shared_ptr<PostgresFlatRepository> repo,
                          std::shared_ptr<Session> session);

    void execute() override;
};