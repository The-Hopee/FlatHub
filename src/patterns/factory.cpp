#include "../include/factory.hpp"

#include "../include/FlatCommand.hpp"
#include "../include/LoginCommand.hpp"
#include "../include/HouseCommand.hpp"
#include "../include/RegisterCommand.hpp"
#include "../include/QuitCommand.hpp"
#include "../include/GetFlatsCommand.hpp"
#include "../include/TakeFlatCommand.hpp"
#include "../include/UpdateFlatStatusCommand.hpp"
#include "../include/CommandsCommand.hpp"

#include "../include/DatabaseManager.hpp"

CommandFactory::CommandFactory(std::shared_ptr<DatabaseManager> db_manager): db_manager_(db_manager)
{
    // здесь если мы хотим новый класс создать ( для регистрации юзера например ) то нам тупо надо добавить пару в мапу
    creators_["/create_flat"] = [this](const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateFlatCommand>(args, db_manager_->getFlatRepo(), session);
    };

    creators_["/login"] = [this](const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateLoginCommand>(args , db_manager_->getUserRepo(), session );
    };

    creators_["/create_house"] = [this](const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateHouseCommand>(args, db_manager_->getHouseRepo(), session);
    };

    creators_["/register"] = [this](const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateRegisterCommand>(args,db_manager_->getUserRepo(), session);
    };

    creators_["/quit"] = [this](const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateQuitCommand>(session);
    };

    creators_["/get_flats"] = [this](const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateGetFlatsCommand>(args, db_manager_->getFlatRepo(), session);
    };

    creators_["/take_flat"] = [this]( const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateTakeFlatCommand>(args, db_manager_->getFlatRepo(), session);
    };

    creators_["/update_flat_status"] = [this]( const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateUpdateFlatStatusCommand>(args, db_manager_->getFlatRepo(),session);
    };
    creators_["/commands"] = [this]( const auto& args, std::shared_ptr<Session> session){
        return std::make_unique<CreateCommandsCommand>(session);
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
    session->do_write("ERROR: Ошибка. Неизвестная комманда!\n");
    return nullptr; // Иначе ноль указатель
}