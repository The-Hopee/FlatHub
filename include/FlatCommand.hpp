#pragma once

#include "command.hpp"
#include "FlatRepository.hpp"

class CreateFlatCommand : public ICommand
{
private:
    size_t house_id, flat_number, price, rooms;

    std::shared_ptr<PostgresFlatRepository> repo_;
public:
    CreateFlatCommand( const std::vector<std::string>&, std::shared_ptr<PostgresFlatRepository> repo );

    void execute() override;
};