#pragma once

#include <unordered_map>
#include <functional>
#include "command.hpp"
#include "DatabaseManager.hpp"

class Session; // forward declaration

class CommandFactory
{
private:
    // словарь: ключ - имя команды, значение - имя функция которая создает объет типа ICommand и возвращает unique_ptr
    // заместо функции у нас будет лямбда которая для каждой команды создает определенный класс ( для логинга свой, для создания юзера свой)
    // таким образом у меня объединение паттернов команда + стратегия, где стратегия выражается через лямбды
    using CreatorFunc = std::function<std::unique_ptr<ICommand>(const std::vector<std::string>&,
                                                                                std::shared_ptr<Session>)>;
    std::unordered_map<std::string, CreatorFunc> creators_;

    std::shared_ptr<DatabaseManager> db_manager_;
public:
    CommandFactory(std::shared_ptr<DatabaseManager> db_manager);

    //фабричный метод
    std::unique_ptr<ICommand> createCommand( const std::string& command, const std::vector<std::string>& args, std::shared_ptr<Session> session );
};