#include "../include/HouseRepository.hpp"
#include <pqxx/pqxx>
#include "../include/logger.hpp"

PostgresHouseRepository::PostgresHouseRepository( const std::string& conn ): conn_str_(conn) {};

void PostgresHouseRepository::saveHouse( const std::string& adress, const std::string& date_of_build, const std::string& builder )
{
    try
    {
        pqxx::connection C(conn_str_);

        pqxx::work W(C);

        std::string sql = "INSERT INTO houses (address, build_year, developer) VALUES ($1, $2, $3)";

        W.exec_params(sql, adress, date_of_build, builder);

        W.commit();

        Logger::Instance().info("HOUSE_REPOSITORY_TRY", "Дом записан в PostgreSQL");   
    }
    catch(const std::exception& e)
    {
        Logger::Instance().error("HOUSE_REPOSITORY_CATCH", "SQL ошибка: " + std::string(e.what()));
    }
    
}