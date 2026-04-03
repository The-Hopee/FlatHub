#include "../include/UpdateFlatStatusCommand.hpp"
#include "../include/logger.hpp"

CreateUpdateFlatStatusCommand::CreateUpdateFlatStatusCommand( const std::vector<std::string>& args, 
std::shared_ptr<PostgresFlatRepository> repo, std::shared_ptr<Session> session ): repo_(repo), session_(session)
{
    if( args.size() != 2 ) throw std::invalid_argument("Некорректное кол-во аргументов!");

    id_flat = std::stoi(args[0]);

    if( args[1] == "approved" || args[1] == "declined" )
    {
        status = args[1];
    }
    else
    {
        throw std::invalid_argument("Аргумент может быть только approved или declined!");
    }
}

void CreateUpdateFlatStatusCommand::execute()
{
    if(session_->getStatusAutorizate() && session_->getCurrentRole() == "moderator")
    {
        bool ans = repo_->UpdateFlatStatus(id_flat, status);

        if( ans )
        {
            Logger::Instance().info("CREATE_UPDATE_FLAT_STATUS_COMMAND","OK: Статус квартиры обновлен!" );
            session_->do_write("OK: Статус квартиры обновлен на " + std::string(status) + "\n");
        }
        else
        {
            Logger::Instance().info("CREATE_UPDATE_FLAT_STATUS_COMMAND","ERROR: Статус квартиры не обновлен!" );
            session_->do_write("ERROR: Квартира не может быть переведа в новый статус: либо не существует, либо уже не в статусе on_moderation!\n");
        }
    }
    else
    {
        Logger::Instance().info("CREATE_TAKE_FLAT_COMMAND","ERROR: Пользователь не авторизован или не является модератором!" );
        session_->do_write("ERROR: Пользователь не авторизован или не является модератором!\n");
    }

    return;
}