#include "../../include/LoginCommand.hpp"
#include "../../include/logger.hpp"

CreateLoginCommand::CreateLoginCommand( const std::vector<std::string>& args, std::shared_ptr<PostgresUserRepository> repo ): repo_(repo)
{
    if( args.size() != 2 ) throw std::invalid_argument("Неверное кол-во аргументов!");
    login    = args[0];
    password = args[1];
}

void CreateLoginCommand::execute()
{
    Logger::Instance().info("CREATE_LOGIN_COMMAND", "Ищем юзера в нашей БД");

    auto ans = repo_->findUserByLogin(login);

    if( !ans )
    {
        Logger::Instance().error("CREATE_LOGIN_COMMAND", "Ошибка. Пользователь с таким логином не найден!");
        return;
    }

    else if( ans->password.compare(password) != 0 )
    {
        Logger::Instance().error("CREATE_LOGIN_COMMAND", "Ошибка. Неверный пароль!");
        return;
    }

    Logger::Instance().info("CREATE_LOGIN_COMMAND", "Успешный вход пользователя с ролью: " + ans->role);

    // Пока просто логируем успех.
    // Позже тут будем обновлять состояние Session (authorized, role, user_id).
}