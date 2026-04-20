cmake_minimum_required(VERSION 3.15)

project(FlatHub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch OpenSSL
FetchContent_Declare(openssl
  URL https://github.com/openssl/openssl/archive/refs/tags/openssl-3.0.0.tar.gz
)
FetchContent_MakeAvailable(openssl)

# Fetch nlohmann/json
FetchContent_Declare(json
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

enable_testing()

include_directories(include)

add_executable(server 
                src/service/http_server.cpp
                src/service/http_session.cpp
                src/service/session.cpp
                src/service/main_server.cpp

                src/auth/jwt_utils.cpp

                src/patterns/logger.cpp
                src/patterns/factory.cpp
                src/patterns/FlatCommand.cpp
                src/patterns/LoginCommand.cpp
                src/patterns/HouseCommand.cpp
                src/patterns/RegisterCommand.cpp
                src/patterns/QuitCommand.cpp
                src/patterns/GetFlatsCommand.cpp
                src/patterns/TakeFlatCommand.cpp
                src/patterns/UpdateFlatStatusCommand.cpp
                src/patterns/CommandsCommand.cpp

                src/repository/FlatRepository.cpp
                src/repository/HouseRepository.cpp
                src/repository/UserRepository.cpp
                )
target_include_directories(server PRIVATE ${PQXX_INCLUDE_DIRS})
target_link_libraries(server PRIVATE
                         Boost::system
                         ${PQXX_LIBRARIES}
                         OpenSSL::SSL
                         OpenSSL::Crypto
                         nlohmann_json::nlohmann_json
                         Threads::Threads)

add_executable(client 
                    src/service/client.cpp
                    src/service/main_client.cpp
                    src/patterns/logger.cpp
                        )
target_link_libraries(client PRIVATE
                             Boost::system
                             Threads::Threads)

# =========================
# TESTS
# =========================
add_executable(tests
                tests/tests.cpp
                src/repository/UserRepository.cpp
                src/repository/FlatRepository.cpp
                src/repository/HouseRepository.cpp
                src/patterns/logger.cpp
                src/auth/jwt_utils.cpp
              )

target_include_directories(tests PRIVATE ${PQXX_INCLUDE_DIRS})
target_link_libraries(tests PRIVATE
                        GTest::gtest
                        GTest::gtest_main
                        ${PQXX_LIBRARIES}
                        Threads::Threads
                        OpenSSL::Crypto
                        OpenSSL::SSL
                        nlohmann_json::nlohmann_json)

add_test(NAME UserRepositoryTests COMMAND tests)