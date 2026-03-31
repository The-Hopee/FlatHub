#include "../include/RegisterCommand.hpp"
#include "../include/logger.hpp"

CreateRegisterCommand::CreateRegisterCommand( const std::vector<std::string>& args,
                                                std::shared_ptr<PostgresUserRepository> repo ): repo_(repo)
{
    if( args.size() != 3 )
    {
        throw std::invalid_argument("Некорректное число аргументов для регистрации!");
    }

    login = args[0];
    password = args[1];
    role = args[2];
}

void CreateRegisterCommand::execute()
{
    Logger::Instance().info("CREATE_REGISTER_COMMAND_EXECUTE", "Создаем юзера и записываем его в бд");

    repo_->saveUser(login, password, role);
}