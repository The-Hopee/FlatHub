#include "../include/FlatRepository.hpp"
#include <pqxx/pqxx>
#include "../include/logger.hpp"

PostgresFlatRepository::PostgresFlatRepository(const std::string& conn): conn_str_(conn) {};

void PostgresFlatRepository::saveFlat( int house_id, int flat_num, int rooms, int price )
{
    try
    {
        pqxx::connection C(conn_str_);
        pqxx::work W(C); // открываем транзакцию

        std::string sql = "INSERT INTO flats (house_id, flat_number, rooms, price) VALUES ($1,$2, $3, $4)";

        W.exec_params(sql, house_id, flat_num, rooms, price);

        W.commit(); // сохраняем в бд

        Logger::Instance().info("FLAT_REPOSITORY_SAVE_FLAT_TRY", "Квартира записана в PostgreSQL");
    }
    catch(const std::exception& e)
    {
        Logger::Instance().error("FLAT_REPOSITORY_SAVE_FLAT_CATCH", "SQL ошибка: " + std::string(e.what()) + "\n");
    }
    
}

std::vector<FlatData> PostgresFlatRepository::getFlatForHouseID( size_t house_id, const std::string& role) const
{
    try
    {
        pqxx::connection C(conn_str_);

        pqxx::work W(C);

        std::string sql;

        if( role == "user" )
        {
            sql = "SELECT id, house_id, flat_number, price, rooms, status FROM flats WHERE house_id = $1 AND status = 'approved'";
        }
        else if( role == "moderator" )
        {
            sql = "SELECT id, house_id, flat_number, price, rooms, status FROM flats WHERE house_id = $1";
        }
        else
        {
            Logger::Instance().info("FLAT_REPOSITORY_GET_TRY", "Такой роли нет в PostgreSQL!\n");
            return {}; 
        }

        pqxx::result R = W.exec_params(sql,house_id);

        if( !R.empty() )
        {
            Logger::Instance().info("FLAT_REPOSITORY_GET_TRY", "Список квартир дома получен\n");

            std::vector<FlatData> ans;

            for( const auto& row: R )
            {
                FlatData temp;

                temp.id         = row[0].as<size_t>();
                temp.house_id   = row[1].as<size_t>();
                temp.flatNumber = row[2].as<size_t>();
                temp.price      = row[3].as<size_t>();
                temp.rooms      = row[4].as<size_t>();
                temp.status     = row[5].as<std::string>();

                ans.push_back(temp);
            }

            return ans;
        }

        Logger::Instance().info("FLAT_REPOSITORY_GET_TRY", "Квартиры для дома не найдены в PostgreSQL!\n");
        return {}; 
    }
    catch(const std::exception& e)
    {
        Logger::Instance().error("FLAT_REPOSITORY_GET_FLAT_CATCH", "SQL ошибка: " + std::string(e.what()) + "\n");
        return {}; 
    }
    
}

bool PostgresFlatRepository::takeFlat( size_t flat_id )
{
    try
    {
        Logger::Instance().info("FLAT_REPOSITORY_TAKE_FLAT_TRY", "Меняем статус...\n");

        pqxx::connection C(conn_str_);

        pqxx::work W(C);

        std::string sql = "UPDATE flats SET status = 'on_moderation' WHERE id = $1 AND status = 'created' RETURNING id";

        pqxx::result R = W.exec_params(sql,flat_id);

        if( !R.empty() )
        {
            Logger::Instance().info("FLAT_REPOSITORY_TAKE_FLAT_TRY", "OK: Квартира переведена в статус on_moderation!");

            W.commit();

            return true;
        }
        else
        {
            Logger::Instance().info("FLAT_REPOSITORY_TAKE_FLAT_TRY", "ERROR: Квартира не может быть взята на модерацию: либо не существует, либо уже не в статусе created!");
            return false;
        }
    }
    catch(const std::exception& e)
    {
        Logger::Instance().error("FLAT_REPOSITORY_TAKE_FLAT_CATCH", "SQL ошибка: " + std::string(e.what()) + "\n");
        return false;
    }   
}

bool PostgresFlatRepository::UpdateFlatStatus( size_t flat_id, const std::string& status )
{
    try
    {
        Logger::Instance().info("FLAT_REPOSITORY_UPDATE_FLAT_STATUS_TRY", "Меняем статус...\n");

        pqxx::connection C(conn_str_);

        pqxx::work W(C);

        std::string sql = "UPDATE flats SET status = $1 WHERE id = $2 AND status = 'on_moderation' RETURNING id";

        pqxx::result R = W.exec_params(sql,status, flat_id);

        if( !R.empty() )
        {
            Logger::Instance().info("FLAT_REPOSITORY_UPDATE_FLAT_STATUS_TRY", "OK: Квартира переведена в статус " + std::string(status) + "!");

            W.commit();

            return true;
        }
        else
        {
            Logger::Instance().info("FLAT_REPOSITORY_TAKE_FLAT_TRY", "ERROR: Квартира не может быть переведа в новый статус: либо не существует, либо уже не в статусе on_moderation!");
            return false;
        }
    }
    catch(const std::exception& e)
    {
        Logger::Instance().error("FLAT_REPOSITORY_TAKE_FLAT_CATCH", "SQL ошибка: " + std::string(e.what()) + "\n");
        return false;
    }
}