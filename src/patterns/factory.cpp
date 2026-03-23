#include "../../include/factory.hpp"
#include "../../include/FlatCommand.hpp"
#include "../../include/LoginCommand.hpp"

CommandFactory::CommandFactory()
{
    // здесь если мы хотим новый класс создать ( для регистрации юзера например ) то нам тупо надо добавить пару в мапу
    creators_["/create_flat"] = [](const auto& args){
        return std::make_unique<CreateFlatCommand>(args);
    };

    creators_["/login"] = [](const auto& args){
        return std::make_unique<CreateLoginCommand>(args);
    };
}

std::unique_ptr<ICommand> CommandFactory::createCommand(const std::string& name, const std::vector<std::string>& args)
{
    auto it = creators_.find(name);
    if( it != creators_.end() )
    {
        return it->second(args); // возвращаем нашу функцию
    }

    return nullptr; // Иначе ноль указатель
}