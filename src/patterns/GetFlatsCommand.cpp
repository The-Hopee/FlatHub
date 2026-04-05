#include "../include/GetFlatsCommand.hpp"
#include "../include/logger.hpp"

CreateGetFlatsCommand::CreateGetFlatsCommand(const std::vector<std::string>& args, 
                            std::shared_ptr<PostgresFlatRepository> repo,
                            std::shared_ptr<Session> session): repo_(repo), session_(session)
{
    if( args.size() != 2 )
    {
        session_->do_write("Неверное кол-во аргументов!\n");
        throw std::invalid_argument("Неверное кол-во аргументов!");
    }

    token = args[0];
    house_id = std::stoi(args[1]);
}

void CreateGetFlatsCommand::execute()
{
    if( session_->getStatusAutorizate() && !session_->getCurrentRole().empty() && session_->checkToken(token) )
    {
        Logger::Instance().info("CREATE_GET_FLATS_COMMAND_EXECUTE", "Ищем квартиры по адресу: " + std::to_string(house_id));

        auto ans = repo_->getFlatForHouseID(house_id, session_->getCurrentRole() );

        if( !ans.empty() )
        {
            Logger::Instance().info("CREATE_GET_FLATS_COMMAND_EXECUTE", "OK Квартиры найдены");
        }
        else
        {
            Logger::Instance().error("CREATE_GET_FLATS_COMMAND_EXECUTE", "ERROR Квартиры не найдены!");
        }

        std::string answer_from_db = "Список квартир: \n";

        for( const auto& elem: ans )
        {
            answer_from_db += "id Квартиры: " + std::to_string(elem.id) + "\n";
            answer_from_db += "Номер квартиры: " + std::to_string(elem.flatNumber) + "\n";
            answer_from_db += "Количество комнат: " + std::to_string(elem.rooms) + "\n";
            answer_from_db += "Стоимость квартиры: " + std::to_string(elem.price) + "\n";
            answer_from_db += "Статус: " + elem.status + "\n";

            answer_from_db += "\n";
        }

        session_->do_write(answer_from_db);

        return;
    }
    else
    {
        Logger::Instance().error("CREATE_GET_FLATS_COMMAND_EXECUTE", "ERROR Пользователь не авторизован или не указаны данные о текущей роли или нет токена!");

        session_->do_write("ERROR: Пользователь не авторизован или не указаны данные о текущей роли или нет токена!\n");

        return;
    }
}