#include <gtest/gtest.h>
#include <pqxx/pqxx>

#include "../include/UserRepository.hpp"
#include "../include/FlatRepository.hpp"

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