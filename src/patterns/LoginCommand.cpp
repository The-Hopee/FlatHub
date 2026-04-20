#include "../../include/LoginCommand.hpp"
#include "../../include/logger.hpp"
#include "../../include/auth/jwt_utils.hpp"

CreateLoginCommand::CreateLoginCommand( const std::vector<std::string>& args, std::shared_ptr<PostgresUserRepository> repo,
std::shared_ptr<Session> session ): repo_(repo), session_(session)
{
    if( args.size() != 2 )
    {
        if (session_) {
            session_->do_write("ERROR: Invalid number of arguments for login!\n");
        }
        throw std::invalid_argument("Invalid number of arguments for login");
    }
    login    = args[0];
    password = args[1];
}

void CreateLoginCommand::execute()
{
    Logger::Instance().info("CREATE_LOGIN_COMMAND", "Searching for user in database: " + login);

    auto ans = repo_->findUserByLogin(login);

    if( !ans )
    {
        Logger::Instance().error("CREATE_LOGIN_COMMAND", "User not found");
        if (session_) {
            session_->do_write("ERROR: User not found!\n");
        }
        return;
    }

    else if( ans->password.compare(password) != 0 )
    {
        Logger::Instance().error("CREATE_LOGIN_COMMAND", "Invalid password");
        if (session_) {
            session_->do_write("ERROR: Invalid password!\n");
        }
        return;
    }

    Logger::Instance().info("CREATE_LOGIN_COMMAND", "Successful login for user: " + ans->login + " with role: " + ans->role);

    if( session_ )
    {
        session_->autorize(true, ans->id, ans->login, ans->role);
        session_->do_write("OK: Login successful, role = " + ans->role + "\n");
        
        // Generate JWT token
        std::string token = JWTUtils::generateToken(ans->id, ans->login, ans->role, 86400);
        session_->do_write("TOKEN: " + token + "\n");
    }
}