#include "../../include/FlatCommand.hpp"
#include "../../include/logger.hpp" 

CreateFlatCommand::CreateFlatCommand( const std::vector<std::string>& args,
std::shared_ptr<PostgresFlatRepository> repo, std::shared_ptr<Session> session ) : repo_(repo), session_(session)
{
    if( args.size() != 5 )
    {
        session_->do_write("Неверное кол-во аргументов!\n");
        throw std::invalid_argument("Неверное кол-во аргументов!");
    }
    token = args[0];
    house_id    = std::stoi(args[1]);
    flat_number = std::stoi(args[2]);
    rooms       = std::stoi(args[3]);
    price       = std::stoi(args[4]);
}

void CreateFlatCommand::execute()
{
    if( session_->getStatusAutorizate() && session_->checkToken(token) )
    {
        Logger::Instance().info("CREATE_FLAT_COMMAND","OK: создаем квартиру с ценой " + std::to_string(price) );
        // здесь выполняется шаг 2: вызывается database manager (Handler::handleCreateFlat) и в нем
        // создается запись в .sql файл бд

        repo_->saveFlat(house_id,flat_number,rooms,price);

        session_->do_write("OK: Пользовательская квартира создана!\n");
    }
    else
    {
        Logger::Instance().info("CREATE_FLAT_COMMAND","ERROR: Пользователь не авторизован или нет токена!" );
        session_->do_write("ERROR: Пользователь не авторизован или нет токена!\n");
    }
}