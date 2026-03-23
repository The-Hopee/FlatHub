#include "../../include/LoginCommand.hpp"
#include "../../include/logger.hpp"

CreateLoginCommand::CreateLoginCommand( const std::vector<std::string>& args )
{
    if( args.size() != 2 ) throw std::invalid_argument("Неверное кол-во аргументов!");
    login    = args[0];
    password = args[1];
}

void CreateLoginCommand::execute()
{
    Logger::Instance().info("CREATE_LOGIN_COMMAND", ("Создаем юзера в базе данных"));
}