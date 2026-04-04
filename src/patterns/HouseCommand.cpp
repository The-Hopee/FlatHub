#include "../include/HouseCommand.hpp"
#include "../include/logger.hpp"

CreateHouseCommand::CreateHouseCommand( const std::vector<std::string>& args, 
std::shared_ptr<PostgresHouseRepository> repo, std::shared_ptr<Session> session ): repo_(repo), session_(session)
{
    if( args.size() != 4 ) throw std::invalid_argument("Некорректное кол-во аргументов!");

    token = args[0];
    adress = args[1];
    date_of_build = args[2];
    if( args.size() == 4 )
    {
        builder = args[3];
    }
    else builder = "none";
}

void CreateHouseCommand::execute()
{
    if( session_->getCurrentRole() == "moderator" && session_->getStatusAutorizate() && session_->checkToken(token) )
    {
        Logger::Instance().info("CREATE_HOUSE_COMMAND","OK: создаем дом по адресу " + adress );
        repo_->saveHouse(adress, date_of_build, builder);

        session_->do_write("OK: Модератор создал дом!\n");
    }
    else
    {
        Logger::Instance().info("CREATE_HOUSE_COMMAND","ERROR: Пользователь не является модератором или не авторизован или нет токена!" );
        session_->do_write("ERROR: Пользователь не является модератором или не авторизован или нет токена!\n");
    }
}