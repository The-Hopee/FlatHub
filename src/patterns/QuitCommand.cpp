#include "../include/QuitCommand.hpp"
#include "../include/logger.hpp"

CreateQuitCommand::CreateQuitCommand(std::shared_ptr<Session> session): session_(session) {}

void CreateQuitCommand::execute()
{
    if( session_->getId() != -1 )
        Logger::Instance().info("CREATE_QUIT_COMMAND_EXECUTE", "Инициирован протокол выхода из сервиса для юзера с id " + std::to_string(session_->getId()));
    else
        Logger::Instance().info("CREATE_QUIT_COMMAND_EXECUTE", "Инициирован протокол выхода из сервиса");

    session_->logout();
}