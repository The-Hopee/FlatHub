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

add_executable(server server.cpp)
target_link_libraries(server PRIVATE Boost::system)

target_include_directories(server PRIVATE ${PQXX_INCLUDE_DIRS})
target_link_libraries(server PRIVATE ${PQXX_LIBRARIES})

add_executable(client client.cpp)
target_link_libraries(client PRIVATE Boost::system)