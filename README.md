# FlatHub

Pet-project backend service for real estate announcements (houses / flats / users) written in **C++17**.

The goal of the project is to practice:
- backend architecture
- asynchronous networking
- PostgreSQL integration
- command-based request handling
- role-based access logic (`user` / `moderator`)
- preparation for middle C++ backend / highload interviews

---

## Stack

- **C++17**
- **Boost.Asio**
- **PostgreSQL**
- **Docker**
- **CMake**

---

## Current architecture

The project is currently split into several logical layers:

### 1. Network layer
Responsible for client/server communication
- `Server`
- `Session`
- `Client`

### 2. Command layer
Responsible for processing user commands
- `ICommand`
- `CreateFlatCommand`
- `CreateHouseCommand`
- `CreateRegisterCommand`
- `CreateLoginCommand`

### 3. Factory layer
Responsible for command creation by command name
- `CommandFactory`

### 4. Database layer
Responsible for persistence and SQL logic
- `FlatRepository`
- `HouseRepository`
- `UserRepository`
- `DatabaseManager`

### 5. Utility layer
- `Logger`

---

## Implemented functionality

### Working now
- create house (`/create_house`)
- create flat (`/create_flat`)
- register user (`/register`)
- login user (`/login`)
- PostgreSQL integration through Docker
- logging to console and file
- command creation through factory
- database manager for repositories

---

## Database schema

Current entities:

### `houses`
- `id`
- `address`
- `build_year`
- `developer`
- `created_at`

### `flats`
- `id`
- `house_id`
- `flat_number`
- `price`
- `rooms`
- `status`

### `users`
- `id`
- `login`
- `password`
- `role`

---

## Current command format

Examples of working commands:

```text
/create_house Pushkina_1 2020 PIK
/create_flat 1 42 3 5000000
/register pasha 12345 user
/register admin 777 moderator
/login pasha 12345

What is already validated
The following flows were manually tested:

House creation
/create_house writes a new row to PostgreSQL
Flat creation
/create_flat writes a new row to PostgreSQL
flat is linked to house through house_id
Registration
/register saves user data into users
Login
successful login works
invalid password is handled
unknown user is handled
Current project status
At this stage the project already supports:

TCP client/server communication
command parsing
command creation through factory
PostgreSQL persistence
house creation
flat creation
user registration
user login
logging system
repository access through DatabaseManager
Key design decisions
1. Command pattern
Each command is represented by a separate class implementing ICommand.

This allows:

easier extension
separation of logic
isolation of business actions
2. Factory method
CommandFactory maps command strings to concrete command objects.

This removes huge if / else if chains from session handling.

3. Repository layer
SQL is isolated from command logic.

Commands do not build SQL directly; they delegate DB work to repositories.

4. DatabaseManager
DatabaseManager is used as a single entry point to repositories:

flat repository
house repository
user repository
This keeps factory construction cleaner.

5. Immediate persistence
Data is saved to the database immediately when command is executed, not at session shutdown.

This avoids:

data loss on disconnect
inconsistent state
delayed persistence bugs
6. House-flat relation
Flats are linked to houses through house_id, not through address string.

Address is business data, while id is the proper technical identifier.

What I learned so far
    C++ / architecture
    how to split server/client/session into .hpp / .cpp
    how to apply Command pattern
    how to apply Factory method
    how to isolate SQL inside repositories
    why object lifetime matters in async/network code
Memory / language
    basics of RAII
    smart pointers (unique_ptr, shared_ptr, weak_ptr)
    difference between std::move and std::forward
    why COW strings were removed from standard implementations
    why shared_ptr control block is thread-safe but object is not
STL / containers
    difference between map and unordered_map
    vector reallocation basics
    size vs capacity
    iterator invalidation basics
Databases
    how to run PostgreSQL in Docker
    how to connect to PostgreSQL from C++ through libpqxx
    how to write INSERT / SELECT
    why transaction logic matters
    why foreign keys matter

Development diary
16 March
    Added docker-compose.yml
    Planned migrations/init.sql
    Started thinking about project architecture and splitting code into .hpp / .cpp
17 March
    Planned split into:
        session
        server
        client
        logger / singleton
    Started reading about:
        allocators
        metaprogramming
18 March
    Split server/client code into .hpp and .cpp
    Started thinking about builder / command-handler style logic
    Continued reading theory and notes
23 March
    Added first classes using Command pattern
    Planned command factory
    Started thinking about:
        login
        registration
        houses
        flats
        database layer
        tokenization and role model
24 March
    Implemented:
        fixed CMakeLists.txt
        added main_server.cpp
        added main_client.cpp
        connected PostgreSQL
    started repository layer
30 March
    Implemented working database flow:
        /create_house
        /create_flat
        logger writes to file and console
        DatabaseManager introduced
    Validated manually through Docker + PostgreSQL console:
        houses are inserted correctly
        flats are inserted correctly
31 March
    Implemented user flow:
        UserRepository
        /register
        /login
    Validated manually:
        user registration writes into PostgreSQL
        successful login works
        invalid password works
        unknown user case works

How to run
    1. Start PostgreSQL through Docker
        Example:

        Bash

        docker run --name db_service -e POSTGRES_PASSWORD=secret -p 5432:5432 -d postgres
    2. Build project
        Bash

        mkdir build
        cd build
        cmake ..
        make
    3. Run server
        Bash

        ./server
    4. Run client
        Bash

        ./client

Manual DB checks
    Examples:
        Houses
        Bash: docker exec -it db_service psql -U postgres -d postgres -c "SELECT * FROM houses;"
        Flats
        Bash: docker exec -it db_service psql -U postgres -d postgres -c "SELECT * FROM flats;"
        Users
        Bash: docker exec -it db_service psql -U postgres -d postgres -c "SELECT * FROM users;"

Notes

This project is developed iteratively:

    1) make it work
    2) make architecture cleaner
    3) improve correctness
    4) improve access control / auth
    5) improve performance and test coverage