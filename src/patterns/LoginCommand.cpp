#include "../../include/LoginCommand.hpp"
#include "../../include/logger.hpp"

CreateLoginCommand::CreateLoginCommand( const std::vector<std::string>& args, std::shared_ptr<PostgresUserRepository> repo,
std::shared_ptr<Session> session ): repo_(repo), session_(session)
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
        session_->do_write("ERROR: Пользователь не найден!\n");
        return;
    }

    else if( ans->password.compare(password) != 0 )
    {
        Logger::Instance().error("CREATE_LOGIN_COMMAND", "Ошибка. Неверный пароль!");
        session_->do_write("ERROR: Неверный пароль!\n");
        return;
    }

    Logger::Instance().info("CREATE_LOGIN_COMMAND", "Успешный вход пользователя с ролью: " + ans->role);

    if( session_ )
    {
        session_->autorize(true, ans->id, ans->login, ans->role);
        session_->do_write("OK: Логин верный, роль = " + ans->role + "\n");
    }
}