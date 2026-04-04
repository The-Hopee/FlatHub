#include "../include/CommandsCommand.hpp"
#include "../include/logger.hpp"

void CreateCommandsCommand::execute()
{
    std::string ans = "Доступные комманды:\n /commands - выводит доступный список команд\n /login {login} {password} - логинит юзера\n /quit - выход из системы\n /register - регистрирует юзера в системе\n /create_flat {house_id} {flat_number} {rooms} {price} - создает квартиру с заданными данными для любой роли\n /create_house {address} {date_of_build} {builder} - создаем дом с данными параметрами. Только для модераторов\n /get_flats {house_id} - выводит квартиры в доме по заданному house_id. Для user - только approved, для moderator - все статусы\n /take_flat {id_flat} - взять квартиру на модерацию. Только для модераторов\n /update_flat_status - {id_flat} {status} - обнонвить статус квартиры (approve or declined). Только для модераторов\n";

    Logger::Instance().info("CREATE_COMMANDS_COMMAND", "Отправляем информацию о доступных коммандах пользователю...");

    session_->do_write(ans);
}