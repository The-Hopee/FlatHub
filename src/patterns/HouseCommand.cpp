#include "../include/HouseCommand.hpp"
#include "../include/logger.hpp"

CreateHouseCommand::CreateHouseCommand( const std::vector<std::string>& args, std::shared_ptr<PostgresHouseRepository> repo ): repo_(repo)
{
    if( args.size() < 2 ) throw std::invalid_argument("Некорректное кол-во аргументов!");

    adress = args[0];
    date_of_build = args[1];
    if( args.size() == 3 )
    {
        builder = args[2];
    }
    else builder = "none";
}

void CreateHouseCommand::execute()
{
    Logger::Instance().info("CREATE_HOUSE_COMMAND",("создаем дом по адресу " + adress) );
    repo_->saveHouse(adress, date_of_build, builder);

}