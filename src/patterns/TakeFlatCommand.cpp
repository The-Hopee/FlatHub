#include "../include/TakeFlatCommand.hpp"
#include "../include/logger.hpp"

CreateTakeFlatCommand::CreateTakeFlatCommand( const std::vector<std::string>& args, 
std::shared_ptr<PostgresFlatRepository> repo, std::shared_ptr<Session> session ): repo_(repo), session_(session)
{
    if( args.size() != 2 ) throw std::invalid_argument("Некорректное кол-во аргументов!");

    token = args[0];
    id_flat = std::stoi(args[1]);
}

void CreateTakeFlatCommand::execute()
{
    if(session_->getStatusAutorizate() && session_->getCurrentRole() == "moderator" && session_->checkToken(token))
    {
        bool ans = repo_->takeFlat(id_flat);

        if( ans )
        {
            Logger::Instance().info("CREATE_TAKE_FLAT_COMMAND","OK: Статус квартиры обновлен!" );
            session_->do_write("OK: Статус квартиры обновлен на 'on_moderation'\n");
        }
        else
        {
            Logger::Instance().info("CREATE_TAKE_FLAT_COMMAND","ERROR: Статус квартиры не обновлен!" );
            session_->do_write("ERROR: Квартира не может быть взята на модерацию: либо не существует, либо уже не в статусе created, либо SQL-ошибка!\n");
        }
    }
    else
    {
        Logger::Instance().info("CREATE_TAKE_FLAT_COMMAND","ERROR: Пользователь не авторизован или не является модератором или нет токена!" );
        session_->do_write("ERROR: Пользователь не авторизован или не является модератором или нет токена!\n");
    }

    return;
}