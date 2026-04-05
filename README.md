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
- create house (`/create_house`) — available only for `moderator`
- create flat (`/create_flat`) — available only for authorized users
- register user (`/register`)
- login user (`/login`)
- get flats by house id (`/get_flats`)
- take flat to moderation (`/take_flat`) — available only for `moderator`
- update flat status (`/update_flat_status`) — available only for `moderator`
- quit session (`/quit`)
- session authorization state
- role-based access control (`user` / `moderator`)
- token-based command validation
- PostgreSQL integration through Docker
- logging to console and file
- command creation through factory
- database manager for repositories
- Google Test integration

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

### Current command format

## Examples of working commands:

```text```
- /create_house Pushkina_1 2020 PIK
- /create_flat 1 42 3 5000000
- /register pasha 12345 user
- /register admin 777 moderator
- /login pasha 12345
- /get_flats 1
- /take_flat 2
- /update_flat_status 2 approved
- /update_flat_status 3 declined
- /quit

## What is already validated
- The following flows were manually tested:`

- House creation
    /create_house writes a new row to PostgreSQL

## Flat creation
- /create_flat writes a new row to PostgreSQL
    flat is linked to house through house_id
- Registration
    /register saves user data into users
- Login
    successful login works
    invalid password is handled
    unknown user is handled
    Current project status
    At this stage the project already supports:

- TCP client/server communication
- command parsing
- command creation through factory
- PostgreSQL persistence
- house creation
- flat creation
- user registration
- user login
- logging system
-repository access through DatabaseManager
- Key design decisions

## Flat moderation
- unauthorized user cannot take flat to moderation
- authorized `user` cannot take flat to moderation
- authorized `moderator` can take flat to moderation
- flat can only be taken when current status is `created`
- flat status can be changed from `on_moderation` to `approved`
- flat status can be changed from `on_moderation` to `declined`
- flat cannot be updated if it is not in `on_moderation`

## Tokenization
- token is generated after successful login
- token is stored in current session
- protected commands require valid token
- token is cleared after `/quit`

## Tests
- repository layer is covered by Google Test integration tests
- tests are executed against separate PostgreSQL test database

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

## What I learned so far
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

- how to store auth-state inside session
- how to propagate session into command handling
- how role-based access control can be enforced at command execution level
- why command execution should depend on current session state
- how to keep auth-state inside session
- how to propagate current session into command execution
- how to enforce role-based access inside commands
- how to implement simple token-based validation
- how to test repository logic with Google Test against isolated PostgreSQL test database
- how to use PostgreSQL transactional logic for moderation flow

## Current project status

At this stage the project already supports:
- TCP client/server communication
- command parsing
- command creation through factory
- PostgreSQL persistence
- house creation
- flat creation
- user registration
- user login
- getting flats by house id
- flat moderation flow
- session authorization state
- token-based validation
- role-based access restrictions
- session quit command
- repository layer tests with Google Test
- logging system
- repository access through `DatabaseManager`

### Access control
- unauthorized user cannot create flat
- unauthorized user cannot create house
- authorized `user` can create flat
- authorized `user` cannot create house
- authorized `moderator` can create house

Development diary
### 16 March
    Added docker-compose.yml
    Planned migrations/init.sql
    Started thinking about project architecture and splitting code into .hpp / .cpp
### 17 March
    Planned split into:
        session
        server
        client
        logger / singleton
    Started reading about:
        allocators
        metaprogramming
### 18 March
    Split server/client code into .hpp and .cpp
    Started thinking about builder / command-handler style logic
    Continued reading theory and notes
### 23 March
    Added first classes using Command pattern
    Planned command factory
    Started thinking about:
        login
        registration
        houses
        flats
        database layer
        tokenization and role model
### 24 March
    Implemented:
        fixed CMakeLists.txt
        added main_server.cpp
        added main_client.cpp
        connected PostgreSQL
    started repository layer
### 30 March
    Implemented working database flow:
        /create_house
        /create_flat
        logger writes to file and console
        DatabaseManager introduced
    Validated manually through Docker + PostgreSQL console:
        houses are inserted correctly
        flats are inserted correctly
### 31 March
    Implemented user flow:
        UserRepository
        /register
        /login
    Validated manually:
        user registration writes into PostgreSQL
        successful login works
        invalid password works
        unknown user case works

### 1 April
Implemented:

    session authorization state
    storing current user info inside session after successful login
    role-based access control for commands

### 2 April
Implemented:

- get_flats

- flat retrieval by `house_id`
- role-based filtering of flat list:
- `user` sees only `approved`
- `moderator` sees all flats

- /quit

Validated manually:
- unauthorized user cannot access flat list
- authorized `user` sees only approved flats
- authorized `moderator` sees all flats
- quit command closes session correctly

### 3 April
Implemented:

- `/take_flat`
- `/update_flat_status`
- moderation flow for flats
- moderator-only access for flat moderation

Validated manually:

- unauthorized user cannot take flat
- regular `user` cannot take flat
- `moderator` can take flat if status is `created`
- flat status can only be updated from `on_moderation`
- approved / declined transitions work correctly

### 4 April
Implemented:

- token generation after successful login
- token storage inside `Session`
- token validation for protected commands
- Google Test integration with separate PostgreSQL test database

Validated manually:

- token is returned after login
- protected commands fail without valid token
- token is cleared on `/quit`

Validated through tests:

- `UserRepository` tests
- `FlatRepository` tests
- 8 passing Google Tests

## Tests

The project includes Google Test integration tests for repository layer.

Currently covered:
- `UserRepository`
- `FlatRepository`

Tests are executed against a separate PostgreSQL test database (`flathub_test`) to avoid modifying the main project database.

### Manual DB checks
    Examples:
        Houses
        Bash: docker exec -it db_service psql -U postgres -d postgres -c "SELECT * FROM houses;"
        Flats
        Bash: docker exec -it db_service psql -U postgres -d postgres -c "SELECT * FROM flats;"
        Users
        Bash: docker exec -it db_service psql -U postgres -d postgres -c "SELECT * FROM users;"

### Notes

This project is developed iteratively:

    1) make it work
    2) make architecture cleaner
    3) improve correctness
    4) improve access control / auth
    5) improve performance and test coverage

## How to run

### 1. Start PostgreSQL in Docker
Run PostgreSQL container:

```bash```
docker run --name db_service -e POSTGRES_PASSWORD=secret -p 5432:5432 -d postgres

If container already exists but is stopped:

```bash```

docker start db_service

### 2. Create database schema
Run SQL manually inside container or use migration file.

### Example manual schema creation:

```bash```

docker exec -it db_service psql -U postgres -d postgres
### Inside PostgreSQL console run:

```sql```

CREATE TABLE houses (
    id SERIAL PRIMARY KEY,
    address VARCHAR(255) NOT NULL,
    build_year INT NOT NULL,
    developer VARCHAR(255),
    created_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE flats (
    id SERIAL PRIMARY KEY,
    house_id INT NOT NULL REFERENCES houses(id),
    flat_number INT NOT NULL,
    price INT NOT NULL,
    rooms INT NOT NULL,
    status VARCHAR(50) DEFAULT 'created'
);

CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    login VARCHAR(255) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    role VARCHAR(255) NOT NULL
);

INSERT INTO houses (address, build_year) VALUES ('ул. Пушкина, д. 1', 2026);

### Exit PostgreSQL console:

```sql```

\q
### 3. Build project
```bash```

mkdir build
cd build
cmake ..
cmake --build .
### 4. Run server
```bash```

./server
### 5. Run client
Open another terminal, go to build and run:

```bash```

./client

### 6. Run tests

```bash```

cd build

./tests
