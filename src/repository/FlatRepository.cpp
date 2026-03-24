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

        Logger::Instance().info("FLAT_REPOSITORY_TRY", "Квартира записана в PostgreSQL");
    }
    catch(const std::exception& e)
    {
        Logger::Instance().error("FLAT_REPOSITORY_CATCH", "SQL ошибка: " + std::string(e.what()) + "\n");
    }
    
}