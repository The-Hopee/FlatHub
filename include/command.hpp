#pragma once

#include <vector>
#include <memory>
#include <iostream>

class ICommand
{
public:
    virtual ~ICommand() = default;

    virtual void execute() = 0;
};
