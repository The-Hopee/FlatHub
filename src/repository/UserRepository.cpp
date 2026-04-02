#include "../include/UserRepository.hpp"
#include "../include/logger.hpp"
#include <pqxx/pqxx>

PostgresUserRepository::PostgresUserRepository( const std::string& conn ): conn_str_(conn) {}

void PostgresUserRepository::saveUser( const std::string& login, const std::string& password, const std::string& role )
{
    try
    {
        pqxx::connection C(conn_str_);

        pqxx::work W(C);

        std::string sql = "INSERT INTO users (login, password, role) VALUES ($1, $2, $3)";

        W.exec_params(sql, login, password, role);

        W.commit();

        Logger::Instance().info("USER_REPOSITORY_SAVE_TRY", "Пользователь записан в PostgreSQL\n");   
    }
    catch(const std::exception& e)
    {
        Logger::Instance().error("USER_REPOSITORY_SAVE_CATCH", "SQL ошибка: " + std::string(e.what()) + "\n");
    }
}

std::optional<UserData> PostgresUserRepository::findUserByLogin( const std::string& login )
{
    try
    {
        pqxx::connection C(conn_str_);

        pqxx::work W(C);

        std::string sql = "SELECT id, login, password, role FROM users WHERE login = $1";

        pqxx::result R = W.exec_params(sql,login);

        if( !R.empty() )
        {
            Logger::Instance().info("USER_REPOSITORY_FIND_TRY", "Пользователь найден в PostgreSQL\n");   
            
            UserData ans;
            
            ans.id       = R[0][0].as<int>();
            ans.login    = R[0][1].as<std::string>();
            ans.password = R[0][2].as<std::string>();
            ans.role     = R[0][3].as<std::string>();

            if (!ans.role.empty() && ans.role.back() == '\n') ans.role.pop_back();

            if (!ans.role.empty() && ans.role.back() == '\r') ans.role.pop_back();

            return ans;
        }

        // W.commit(); на будущее: для SELECT commit не обязателен
    
        Logger::Instance().info("USER_REPOSITORY_FIND_TRY", "Пользователь не найден в PostgreSQL\n");
        return std::nullopt;   
    }
    catch(const std::exception& e)
    {
        Logger::Instance().error("USER_REPOSITORY_FIND_CATCH", "SQL ошибка: " + std::string(e.what()) + "\n");
        return std::nullopt;
    }   
}