#include "../include/RegisterCommand.hpp"
#include "../include/logger.hpp"

CreateRegisterCommand::CreateRegisterCommand( const std::vector<std::string>& args,
                                                std::shared_ptr<PostgresUserRepository> repo,
                                                std::shared_ptr<Session> session ): repo_(repo), session_(session)
{
    if( args.size() != 3 )
    {
        session_->do_write("Некорректное число аргументов для регистрации!");
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