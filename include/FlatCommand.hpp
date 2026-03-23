#pragma once

#include "command.hpp"

class CreateFlatCommand : public ICommand
{
private:
    size_t house_id, flat_number, price, rooms;
public:
    CreateFlatCommand( const std::vector<std::string>& );

    void execute() override;
};