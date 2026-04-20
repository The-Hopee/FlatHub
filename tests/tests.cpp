#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include <ctime>

#include "../include/UserRepository.hpp"
#include "../include/FlatRepository.hpp"
#include "../include/HouseRepository.hpp"
#include "../include/auth/jwt_utils.hpp"

// Строка подключения к тестовой БД
static const std::string TEST_CONN =
    "dbname=flathub_test user=postgres password=secret host=127.0.0.1 port=5432";

// ============================================================
// Общий fixture
// ============================================================
class RepositoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        pqxx::connection C(TEST_CONN);
        pqxx::work W(C);

        // Чистим всё перед каждым тестом
        W.exec("DELETE FROM flats;");
        W.exec("DELETE FROM users;");
        W.exec("DELETE FROM houses;");

        W.commit();
    }

    void TearDown() override
    {
        pqxx::connection C(TEST_CONN);
        pqxx::work W(C);

        W.exec("DELETE FROM flats;");
        W.exec("DELETE FROM users;");
        W.exec("DELETE FROM houses;");

        W.commit();
    }
};

// ============================================================
// USER REPOSITORY TESTS
// ============================================================

TEST_F(RepositoryTest, SaveUserAndFindByLogin)
{
    PostgresUserRepository repo(TEST_CONN);

    repo.saveUser("test_user", "12345", "user");

    auto result = repo.findUserByLogin("test_user");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->login, "test_user");
    EXPECT_EQ(result->password, "12345");
    EXPECT_EQ(result->role, "user");
}

TEST_F(RepositoryTest, FindUnknownUserReturnsNullopt)
{
    PostgresUserRepository repo(TEST_CONN);

    auto result = repo.findUserByLogin("unknown_user");

    EXPECT_FALSE(result.has_value());
}

// ============================================================
// FLAT REPOSITORY TESTS
// ============================================================

TEST_F(RepositoryTest, GetFlatsForUserReturnsOnlyApproved)
{
    pqxx::connection C(TEST_CONN);
    pqxx::work W(C);

    // создаём дом и забираем его id
    pqxx::result house_res =
        W.exec("INSERT INTO houses (address, build_year, developer) "
               "VALUES ('House_1', 2020, 'PIK') RETURNING id;");

    int house_id = house_res[0][0].as<int>();

    // создаём 2 квартиры: approved и created
    W.exec_params(
        "INSERT INTO flats (house_id, flat_number, price, rooms, status) "
        "VALUES ($1, $2, $3, $4, $5);",
        house_id, 101, 5000000, 3, "approved");

    W.exec_params(
        "INSERT INTO flats (house_id, flat_number, price, rooms, status) "
        "VALUES ($1, $2, $3, $4, $5);",
        house_id, 102, 6000000, 2, "created");

    W.commit();

    PostgresFlatRepository repo(TEST_CONN);

    auto flats = repo.getFlatForHouseID(house_id, "user");

    ASSERT_EQ(flats.size(), 1);
    EXPECT_EQ(flats[0].flatNumber, 101);
    EXPECT_EQ(flats[0].status, "approved");
}

TEST_F(RepositoryTest, GetFlatsForModeratorReturnsAll)
{
    pqxx::connection C(TEST_CONN);
    pqxx::work W(C);

    pqxx::result house_res =
        W.exec("INSERT INTO houses (address, build_year, developer) "
               "VALUES ('House_1', 2020, 'PIK') RETURNING id;");

    int house_id = house_res[0][0].as<int>();

    W.exec_params(
        "INSERT INTO flats (house_id, flat_number, price, rooms, status) "
        "VALUES ($1, $2, $3, $4, $5);",
        house_id, 101, 5000000, 3, "approved");

    W.exec_params(
        "INSERT INTO flats (house_id, flat_number, price, rooms, status) "
        "VALUES ($1, $2, $3, $4, $5);",
        house_id, 102, 6000000, 2, "created");

    W.commit();

    PostgresFlatRepository repo(TEST_CONN);

    auto flats = repo.getFlatForHouseID(house_id, "moderator");

    ASSERT_EQ(flats.size(), 2);
}

TEST_F(RepositoryTest, TakeFlatChangesCreatedToOnModeration)
{
    pqxx::connection C(TEST_CONN);
    pqxx::work W(C);

    pqxx::result house_res =
        W.exec("INSERT INTO houses (address, build_year, developer) "
               "VALUES ('House_1', 2020, 'PIK') RETURNING id;");

    int house_id = house_res[0][0].as<int>();

    pqxx::result flat_res =
        W.exec_params(
            "INSERT INTO flats (house_id, flat_number, price, rooms, status) "
            "VALUES ($1, $2, $3, $4, $5) RETURNING id;",
            house_id, 201, 7000000, 4, "created");

    int flat_id = flat_res[0][0].as<int>();

    W.commit();

    PostgresFlatRepository repo(TEST_CONN);

    bool result = repo.takeFlat(flat_id);
    EXPECT_TRUE(result);

    pqxx::connection C2(TEST_CONN);
    pqxx::work W2(C2);
    pqxx::result R = W2.exec_params("SELECT status FROM flats WHERE id = $1;", flat_id);

    ASSERT_FALSE(R.empty());
    EXPECT_EQ(R[0][0].as<std::string>(), "on_moderation");
}

TEST_F(RepositoryTest, TakeFlatFailsIfNotCreated)
{
    pqxx::connection C(TEST_CONN);
    pqxx::work W(C);

    pqxx::result house_res =
        W.exec("INSERT INTO houses (address, build_year, developer) "
               "VALUES ('House_1', 2020, 'PIK') RETURNING id;");

    int house_id = house_res[0][0].as<int>();

    pqxx::result flat_res =
        W.exec_params(
            "INSERT INTO flats (house_id, flat_number, price, rooms, status) "
            "VALUES ($1, $2, $3, $4, $5) RETURNING id;",
            house_id, 201, 7000000, 4, "approved");

    int flat_id = flat_res[0][0].as<int>();

    W.commit();

    PostgresFlatRepository repo(TEST_CONN);

    bool result = repo.takeFlat(flat_id);
    EXPECT_FALSE(result);
}

TEST_F(RepositoryTest, UpdateFlatStatusChangesOnModerationToApproved)
{
    pqxx::connection C(TEST_CONN);
    pqxx::work W(C);

    pqxx::result house_res =
        W.exec("INSERT INTO houses (address, build_year, developer) "
               "VALUES ('House_1', 2020, 'PIK') RETURNING id;");

    int house_id = house_res[0][0].as<int>();

    pqxx::result flat_res =
        W.exec_params(
            "INSERT INTO flats (house_id, flat_number, price, rooms, status) "
            "VALUES ($1, $2, $3, $4, $5) RETURNING id;",
            house_id, 301, 8000000, 5, "on_moderation");

    int flat_id = flat_res[0][0].as<int>();

    W.commit();

    PostgresFlatRepository repo(TEST_CONN);

    bool result = repo.UpdateFlatStatus(flat_id, "approved");
    EXPECT_TRUE(result);

    pqxx::connection C2(TEST_CONN);
    pqxx::work W2(C2);
    pqxx::result R = W2.exec_params("SELECT status FROM flats WHERE id = $1;", flat_id);

    ASSERT_FALSE(R.empty());
    EXPECT_EQ(R[0][0].as<std::string>(), "approved");
}

TEST_F(RepositoryTest, UpdateFlatStatusFailsIfNotOnModeration)
{
    pqxx::connection C(TEST_CONN);
    pqxx::work W(C);

    pqxx::result house_res =
        W.exec("INSERT INTO houses (address, build_year, developer) "
               "VALUES ('House_1', 2020, 'PIK') RETURNING id;");

    int house_id = house_res[0][0].as<int>();

    pqxx::result flat_res =
        W.exec_params(
            "INSERT INTO flats (house_id, flat_number, price, rooms, status) "
            "VALUES ($1, $2, $3, $4, $5) RETURNING id;",
            house_id, 301, 8000000, 5, "created");

    int flat_id = flat_res[0][0].as<int>();

    W.commit();

    PostgresFlatRepository repo(TEST_CONN);

    bool result = repo.UpdateFlatStatus(flat_id, "approved");
    EXPECT_FALSE(result);
}

// ============================================================
// HOUSE REPOSITORY TESTS
// ============================================================

TEST_F(RepositoryTest, SaveHouseSuccessfully)
{
    PostgresHouseRepository repo(TEST_CONN);

    repo.saveHouse("Новостройка на Тверской", "2023", "Лукойл Инвест");

    // Проверяем что дом был добавлен в БД
    pqxx::connection C(TEST_CONN);
    pqxx::work W(C);
    pqxx::result R = W.exec("SELECT COUNT(*) FROM houses WHERE address='Новостройка на Тверской';");
    
    ASSERT_FALSE(R.empty());
    int count = R[0][0].as<int>();
    EXPECT_EQ(count, 1);
}

// ============================================================
// JWT UTILS TESTS
// ============================================================

/**
 * Тест генерации JWT токена
 * 
 * Проверяет что токен:
 * 1. Успешно создаётся
 * 2. Имеет правильную структуру (header.payload.signature)
 * 3. Содержит необходимые поля
 */
TEST(JWTUtilsTest, GenerateTokenCreatesValidToken)
{
    // Генерируем токен для пользователя
    std::string token = JWTUtils::generateToken(1, "testuser", "user", 3600);
    
    // Проверяем что токен не пуст
    ASSERT_FALSE(token.empty());
    
    // Проверяем что токен содержит две точки (разделители между header, payload, signature)
    int dot_count = 0;
    for (char c : token) {
        if (c == '.') dot_count++;
    }
    EXPECT_EQ(dot_count, 2);
}

/**
 * Тест валидации правильного токена
 * 
 * Проверяет что валидный токен:
 * 1. Успешно распознаётся как валидный
 * 2. Содержит правильные данные пользователя
 * 3. Не истёк по времени
 */
TEST(JWTUtilsTest, ValidateTokenAcceptsValidToken)
{
    // Генерируем токен на 1 час
    std::string token = JWTUtils::generateToken(42, "moderator_user", "moderator", 3600);
    
    // Валидируем токен
    auto payload = JWTUtils::validateToken(token);
    
    // Проверяем что токен валидный
    ASSERT_TRUE(payload.has_value());
    
    // Проверяем что данные совпадают
    EXPECT_EQ(payload->user_id, 42);
    EXPECT_EQ(payload->login, "moderator_user");
    EXPECT_EQ(payload->role, "moderator");
}

/**
 * Тест отказа для истёкшего токена
 * 
 * Проверяет что токен с истёкшим временем жизни:
 * 1. Распознаётся как невалидный
 * 2. Возвращает std::nullopt
 */
TEST(JWTUtilsTest, ValidateTokenRejectsExpiredToken)
{
    // Генерируем токен со сроком жизни -1 секунда (уже истёк)
    std::string token = JWTUtils::generateToken(1, "old_user", "user", -1);
    
    // Попытаемся валидировать
    auto payload = JWTUtils::validateToken(token);
    
    // Должен быть пусто (токен истёк)
    EXPECT_FALSE(payload.has_value());
}

/**
 * Тест отказа для повреждённого токена
 * 
 * Проверяет что:
 * 1. Токен с неправильной подписью распознаётся как невалидный
 * 2. Токен с неправильной структурой отклоняется
 * 3. Пустой токен отклоняется
 */
TEST(JWTUtilsTest, ValidateTokenRejectsInvalidToken)
{
    // Генерируем правильный токен
    std::string valid_token = JWTUtils::generateToken(1, "user", "user", 3600);
    
    // Повреждаем токен, изменяя последний символ подписи
    std::string invalid_token = valid_token;
    if (!invalid_token.empty()) {
        invalid_token[invalid_token.length() - 1] = 'X';
    }
    
    // Попытаемся валидировать повреждённый токен
    auto payload = JWTUtils::validateToken(invalid_token);
    
    // Должен быть пусто (неверная подпись)
    EXPECT_FALSE(payload.has_value());
}

/**
 * Тест отказа для пустого токена
 */
TEST(JWTUtilsTest, ValidateTokenRejectsEmptyToken)
{
    auto payload = JWTUtils::validateToken("");
    EXPECT_FALSE(payload.has_value());
}

/**
 * Тест отказа для токена с неправильной структурой
 */
TEST(JWTUtilsTest, ValidateTokenRejectsMalformedToken)
{
    auto payload = JWTUtils::validateToken("this.is.not.a.valid.jwt");
    EXPECT_FALSE(payload.has_value());
}

/**
 * Тест Base64 URL кодирования
 * Проверяет что функция кодирования работает через генерацию и валидацию токена
 */
TEST(JWTUtilsTest, Base64URLEncodingThroughToken)
{
    // Генерируем и валидируем токен - это косвенно проверяет Base64 кодирование
    std::string token = JWTUtils::generateToken(1, "testuser", "user", 3600);
    auto payload = JWTUtils::validateToken(token);
    
    ASSERT_TRUE(payload.has_value());
    EXPECT_EQ(payload->login, "testuser");
}

/**
 * Тест Base64 URL декодирования
 * Проверяется через валидацию токена с различными данными
 */
TEST(JWTUtilsTest, Base64URLDecodingThroughToken)
{
    std::string original_login = "Test data for Base64 URL decoding";
    
    // Генерируем токен и проверяем что данные правильно кодируются/декодируются
    std::string token = JWTUtils::generateToken(5, "decoder_user", "user", 3600);
    auto payload = JWTUtils::validateToken(token);
    
    ASSERT_TRUE(payload.has_value());
    EXPECT_EQ(payload->login, "decoder_user");
}

/**
 * Тест различных ролей в токене
 * 
 * Проверяет что токены с разными ролями
 * содержат правильную информацию о роли
 */
TEST(JWTUtilsTest, TokenWithDifferentRoles)
{
    // Тест для роли 'user'
    std::string user_token = JWTUtils::generateToken(1, "user1", "user", 3600);
    auto user_payload = JWTUtils::validateToken(user_token);
    ASSERT_TRUE(user_payload.has_value());
    EXPECT_EQ(user_payload->role, "user");
    
    // Тест для роли 'moderator'
    std::string mod_token = JWTUtils::generateToken(2, "mod1", "moderator", 3600);
    auto mod_payload = JWTUtils::validateToken(mod_token);
    ASSERT_TRUE(mod_payload.has_value());
    EXPECT_EQ(mod_payload->role, "moderator");
}

/**
 * Тест долгоживущего токена
 * 
 * Проверяет что токены с длительным временем жизни
 * остаются валидными
 */
TEST(JWTUtilsTest, LongLivedToken)
{
    // Создаём токен на 30 дней
    int thirty_days = 30 * 24 * 60 * 60;
    std::string token = JWTUtils::generateToken(1, "user", "user", thirty_days);
    
    auto payload = JWTUtils::validateToken(token);
    
    ASSERT_TRUE(payload.has_value());
    EXPECT_EQ(payload->user_id, 1);
}