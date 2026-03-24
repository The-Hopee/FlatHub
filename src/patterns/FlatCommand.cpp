#include "../../include/FlatCommand.hpp"
#include "../../include/logger.hpp" 

CreateFlatCommand::CreateFlatCommand( const std::vector<std::string>& args, std::shared_ptr<PostgresFlatRepository> repo ) : repo_(repo)
{
    if( args.size() != 4 ) throw std::invalid_argument("Неверное кол-во аргументов!");
    house_id    = std::stoi(args[0]);
    flat_number = std::stoi(args[1]);
    rooms       = std::stoi(args[2]);
    price       = std::stoi(args[3]);
}

void CreateFlatCommand::execute()
{
    Logger::Instance().info("CREATE_FLAT_COMMAND",("создаем квартиру с ценой " + std::to_string(price)) );
    // здесь выполняется шаг 2: вызывается database manager (Handler::handleCreateFlat) и в нем
    // создается запись в .sql файл бд

    repo_->saveFlat(house_id,flat_number,rooms,price);
}