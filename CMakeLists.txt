cmake_minimum_required(VERSION 3.15)

project(FlatHub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENT system)
find_package(Threads REQUIRED)
find_package(PQXX REQUIRED libpqxx)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(include)

add_executable(server 
                src/service/server.cpp
                src/service/main_server.cpp
                src/service/session.cpp
                src/patterns/logger.cpp
                src/patterns/factory.cpp
                src/pattenrs/FlatCommand.cpp
                src/patterns/LoginCommand.cpp
                #src/repository/FlatResository.cpp
                )
target_include_directories(server PRIVATE ${PQXX_INCLUDE_DIRS})
target_link_libraries(server PRIVATE
                         Boost::system
                         ${PQXX_LIBRARIES}
                         Threads::Threads)

add_executable(client 
                    src/service/client.cpp
                    src/service/main_client.cpp
                        )
target_link_libraries(client PRIVATE
                             Boost::system
                             Threads::Threads)