#include "../../include/factory.hpp"
#include "../../include/FlatCommand.hpp"
#include "../../include/LoginCommand.hpp"

CommandFactory::CommandFactory(std::shared_ptr<PostgresFlatRepository> repo): repo_(repo)
{
    // здесь если мы хотим новый класс создать ( для регистрации юзера например ) то нам тупо надо добавить пару в мапу
    creators_["/create_flat"] = [this](const auto& args){
        return std::make_unique<CreateFlatCommand>(args, this->repo_);
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