#include "../include/factory.hpp"

#include "../include/FlatCommand.hpp"
#include "../include/LoginCommand.hpp"
#include "../include/HouseCommand.hpp"
#include "../include/RegisterCommand.hpp"

#include "../include/DatabaseManager.hpp"

CommandFactory::CommandFactory(std::shared_ptr<DatabaseManager> db_manager): db_manager_(db_manager)
{
    // здесь если мы хотим новый класс создать ( для регистрации юзера например ) то нам тупо надо добавить пару в мапу
    creators_["/create_flat"] = [this](const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateFlatCommand>(args, db_manager_->getFlatRepo());
    };

    creators_["/login"] = [this](const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateLoginCommand>(args , db_manager_->getUserRepo(), session );
    };

    creators_["/create_house"] = [this](const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateHouseCommand>(args, db_manager_->getHouseRepo());
    };

    creators_["/register"] = [this]( const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateRegisterCommand>(args,db_manager_->getUserRepo());
    };
}

std::unique_ptr<ICommand> CommandFactory::createCommand(const std::string& name,
                                                         const std::vector<std::string>& args,
                                                          std::shared_ptr<Session> session)
{
    auto it = creators_.find(name);
    if( it != creators_.end() )
    {
        return it->second(args,session); // возвращаем нашу функцию
    }

    return nullptr; // Иначе ноль указатель
}