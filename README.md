# FlatHub

Pet-project backend service for real estate announcements (houses / flats / users) written in **C++17**.

The goal of the project is to practice:
- backend architecture
- asynchronous networking with HTTP/REST
- JWT token-based authentication
- PostgreSQL integration
- command-based request handling
- role-based access logic (`user` / `moderator`)
- horizontal scalability through stateless design
- preparation for middle C++ backend / highload interviews

---

## Stack

- **C++17**
- **Boost.Asio** (async I/O)
- **Boost.Beast** (HTTP server)
- **OpenSSL** (JWT signing, TLS support)
- **nlohmann/json** (JSON parsing/generation)
- **PostgreSQL** (persistence)
- **Docker** (database)
- **CMake** (build system)

---

## Current Architecture

FlatHub v2 features a **modernized REST API with JWT authentication**:

### 1. Network Layer (HTTP/REST with Boost.Beast)
Handles REST API requests over HTTP
- `HttpServer`: Accepts connections, manages lifecycle
- `HttpSession`: Processes HTTP requests, handles JSON payloads
- All communication is stateless (suitable for horizontal scaling)

**Key Improvement:** Replaced TCP socket protocol with industry-standard REST API + JSON

### 2. Authentication Layer (JWT + OpenSSL)
Implements stateless, token-based authentication
- `JWTUtils`: Token generation and validation using HS256 (HMAC-SHA256)
- Tokens include user claims: `user_id`, `login`, `role`, expiration time
- Tokens are validated on each protected request via `Authorization: Bearer <token>` header
- 24-hour default token expiry (configurable)

**Key Benefit:** Enables horizontal scaling without session affinity

### 3. Command Layer (Unchanged)
Backward-compatible command pattern for business logic
- `ICommand` interface
- Concrete commands: `CreateFlatCommand`, `LoginCommand`, `HouseCommand`, etc.
- Commands are referenced but not used in HTTP layer (commands were designed for TCP protocol)

### 4. Database Layer (Unchanged)
SQL isolation through repositories
- `PostgresFlatRepository`: Flat CRUD + role-based filtering
- `PostgresHouseRepository`: House CRUD
- `PostgresUserRepository`: User CRUD + login validation
- `DatabaseManager`: Single entry point to all repositories

### 5. Utility Layer (Unchanged)
- `Logger`: File and console logging

---

## REST API Endpoints

### Authentication Endpoints

#### POST `/api/login`
Login with credentials, receive JWT token
```bash
curl -X POST http://localhost:8080/api/login \
  -H "Content-Type: application/json" \
  -d '{
    "login": "user@example.com",
    "password": "securepassword"
  }'
```
**Response:**
```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "role": "user",
  "message": "Login successful"
}
```

#### POST `/api/register`
Register a new user
```bash
curl -X POST http://localhost:8080/api/register \
  -H "Content-Type: application/json" \
  -d '{
    "login": "newuser@example.com",
    "password": "securepassword",
    "role": "user"
  }'
```

### Protected Endpoints (require Bearer token)

#### POST `/api/flat`
Create a new flat (requires valid JWT token)
```bash
curl -X POST http://localhost:8080/api/flat \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <token>" \
  -d '{
    "house_id": 1,
    "flat_number": 101,
    "rooms": 3,
    "price": 5000000
  }'
```

#### GET `/api/flats/{house_id}`
Get flats for a specific house (public, no auth required)
```bash
curl http://localhost:8080/api/flats/1
```

#### POST `/api/house`
Create a new house (moderator only)
```bash
curl -X POST http://localhost:8080/api/house \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <token>" \
  -d '{
    "address": "123 Main St, City",
    "build_year": 2024,
    "developer": "Developer Inc"
  }'
```

#### POST `/api/flat/take`
Take a flat for moderation (moderator only)
```bash
curl -X POST http://localhost:8080/api/flat/take \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <token>" \
  -d '{"flat_id": 42}'
```

#### PUT `/api/flat/status`
Update flat status (moderator only)
```bash
curl -X PUT http://localhost:8080/api/flat/status \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <token>" \
  -d '{
    "flat_id": 42,
    "status": "approved"
  }'
```

---

## Implemented functionality

### Working now (REST API)
- **POST** `/api/login` — authenticate user, receive JWT token
- **POST** `/api/register` — register new user
- **POST** `/api/flat` — create flat (requires token)
- **GET** `/api/flats/{id}` — get flats for house (role-based filtering)
- **POST** `/api/house` — create house (moderator only, requires token)
- **POST** `/api/flat/take` — take flat for moderation (moderator only)
- **PUT** `/api/flat/status` — update flat status (moderator only)
- **JWT token validation** — token expiry, signature verification
- **Role-based access control** — user/moderator separation
- **PostgreSQL persistence** — immediate data storage
- **JSON request/response** — standard REST API format
- **Horizontal scalability** — stateless design via JWT tokens
- **Logging** — to console and file (`logs/app.log`)

---

## Database schema

Current entities:

### `houses`
- `id` (PRIMARY KEY)
- `address` (VARCHAR)
- `build_year` (INT)
- `developer` (VARCHAR)
- `created_at` (TIMESTAMP)

### `flats`
- `id` (PRIMARY KEY)
- `house_id` (FOREIGN KEY → houses.id)
- `flat_number` (INT)
- `price` (INT)
- `rooms` (INT)
- `status` (VARCHAR) — values: `created`, `on_moderation`, `approved`, `declined`

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

## Testing the Application

The application includes both automated tests (Google Test) and manual testing via HTTP API.

### Automated Tests
The project includes Google Test integration tests for the repository layer:

Currently covered:
- `UserRepository` (user CRUD operations)
- `FlatRepository` (flat CRUD + role-based filtering)

Tests are executed against a separate PostgreSQL test database (`flathub_test`).

#### Run tests:
```bash
cd build
ctest  # or: ./tests
```

### Manual Database Inspection
Query the database directly:

```bash
# View houses
docker exec -it db_service psql -U postgres -d flat_hub_db -c "SELECT * FROM houses;"

# View flats
docker exec -it db_service psql -U postgres -d flat_hub_db -c "SELECT * FROM flats;"

# View users
docker exec -it db_service psql -U postgres -d flat_hub_db -c "SELECT * FROM users;"
```

### Manual HTTP API Testing

Use `curl` or Postman to test the REST API endpoints:

```bash
# 1. Register a new user
curl -X POST http://localhost:8080/api/register \
  -H "Content-Type: application/json" \
  -d '{
    "login": "alice",
    "password": "password123",
    "role": "user"
  }'

# 2. Login to get JWT token
TOKEN=$(curl -X POST http://localhost:8080/api/login \
  -H "Content-Type: application/json" \
  -d '{"login":"alice","password":"password123"}' \
  | jq -r '.token')

echo "Your JWT token: $TOKEN"

# 3. Create a house (moderator only - register as moderator first)
curl -X POST http://localhost:8080/api/house \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "address": "123 Main Street, Downtown",
    "build_year": 2024,
    "developer": "BuildCorp Inc"
  }'

# 4. Create a flat
curl -X POST http://localhost:8080/api/flat \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "house_id": 1,
    "flat_number": 101,
    "rooms": 3,
    "price": 5000000
  }'

# 5. Get flats for a house
curl http://localhost:8080/api/flats/1 | jq .

# 6. Take flat for moderation (moderator only)
curl -X POST http://localhost:8080/api/flat/take \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"flat_id": 1}'

# 7. Update flat status (moderator only)
curl -X PUT http://localhost:8080/api/flat/status \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "flat_id": 1,
    "status": "approved"
  }'
```

#### JWT Token Format

The JWT token returned from `/api/login` contains:
- **Header:** Algorithm (HS256), Token type (JWT)
- **Payload:** user_id, login, role, issued_at (iat), expiration (exp)
- **Signature:** HMAC-SHA256 signed with secret key

Example decoded token:
```json
{
  "alg": "HS256",
  "typ": "JWT"
}
{
  "user_id": 1,
  "login": "alice",
  "role": "user",
  "iat": 1713607274,
  "exp": 1713693674
}
```

**Token Expiration:** 24 hours (configurable in `JWTUtils::generateToken()`)

### Monitoring

Check the application logs:
```bash
tail -f logs/app.log
```

Logs include:
- Server startup information
- Request logging (endpoint, method, user)
- Authentication events (login, token validation)
- Error messages
- Database operation logging

---

## How to Build and Run

### 1. Start PostgreSQL in Docker

Using docker-compose (recommended):
```bash
docker-compose up -d
```

Or manually:
```bash
docker run --name db_service \
  -e POSTGRES_PASSWORD=secret \
  -p 5432:5432 \
  -d postgres:15
```

If container exists but is stopped:
```bash
docker start db_service
```

### 2. Create Database Schema

Option A: Using migration file (auto-runs with docker-compose):
```bash
docker exec db_service psql -U postgres -d flat_hub_db < migrations/001_init.sql
```

Option B: Manual creation:
```bash
docker exec -it db_service psql -U postgres -d postgres
```

Inside PostgreSQL console:
```sql
CREATE DATABASE flat_hub_db;
CREATE DATABASE flathub_test;

\c flat_hub_db

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

-- Sample data
INSERT INTO houses (address, build_year, developer) 
VALUES ('123 Main St', 2024, 'BuildCorp');
```

Exit with: `\q`

### 3. Build Project

```bash
cd /path/to/FlatHub
mkdir build
cd build
cmake ..
cmake --build .
```

**Dependencies that must be installed:**
- `libpqxx-dev` (PostgreSQL C++ driver)
- `libboost-all-dev` (Boost libraries)
- `libssl-dev` (OpenSSL)
- `cmake` (build system)
- `g++` (C++ compiler, C++17 support required)

**Ubuntu/Debian:**
```bash
sudo apt-get install -y \
  libpqxx-dev \
  libboost-all-dev \
  libssl-dev \
  cmake \
  g++
```

### 4. Run Server

```bash
cd build
./server
```

Expected output:
```
[2026-04-20 00:01:14] [MAIN_TRY] [INFO] Starting HTTP server with JWT authentication...
[2026-04-20 00:01:14] [HTTP_SERVER] [INFO] HTTP Server initialized on port 8080
[2026-04-20 00:01:14] [MAIN_TRY] [INFO] HTTP Server started on port 8080. Listening for requests...
[2026-04-20 00:01:14] [MAIN_TRY] [INFO] Available endpoints: /api/login, /api/register, /api/flat, /api/flats/{id}, /api/house
```

Server is now listening on `http://localhost:8080`

### 5. Run Tests

```bash
cd build
ctest
```

Or run directly:
```bash
./tests
```

### 6. Run Legacy TCP Client (Optional)

If you want to test the old TCP protocol (for backward compatibility):
```bash
cd build
./client
```

Then enter commands like:
```
/login username password
/register username password
/create_house address build_year developer
/create_flat house_id flat_number rooms price
```

---

## Architecture Notes

### Stateless Design for Horizontal Scaling

Unlike the old TCP-based system, the new HTTP server is **stateless**:

1. **No session affinity needed**: Each request includes the JWT token, so any server instance can process it
2. **Can be deployed behind a load balancer**: Multiple instances behind nginx/haproxy
3. **Easy horizontal scaling**: Add more instances as traffic grows
4. **No session storage needed**: Tokens are self-contained and cryptographically signed

Example deployment with 3 instances:
```
Load Balancer (nginx)
    ↓
    ├→ HTTP Server (port 8080)
    ├→ HTTP Server (port 8080)
    └→ HTTP Server (port 8080)
        ↓
    PostgreSQL (single database)
```

### Security Considerations

**Current Implementation (Development):**
- JWT secret key is hardcoded in `jwt_utils.cpp` (for demo purposes)
- No HTTPS/TLS configured
- Passwords stored as plain text

**Production Recommendations:**
1. **Load JWT secret from environment variable:**
   ```cpp
   const std::string SECRET_KEY = std::getenv("JWT_SECRET");
   ```

2. **Enable TLS/HTTPS:**
   - Use Boost.Beast SSL streams
   - Configure certificates in `HttpServer`

3. **Hash passwords:**
   - Use bcrypt or Argon2 in `PostgresUserRepository::saveUser()`
   - Verify hashes in `LoginCommand::execute()`

4. **Rate limiting:**
   - Implement token bucket algorithm on login endpoint
   - Prevent brute force attacks

5. **Token revocation:**
   - Maintain a blacklist of revoked tokens
   - Useful for logout / security incidents

---

## Key Files

| File | Purpose |
|------|---------|
| `include/http_server.hpp` / `src/service/http_server.cpp` | HTTP server using Boost.Beast |
| `include/http_session.hpp` / `src/service/http_session.cpp` | HTTP request handler |
| `include/auth/jwt_utils.hpp` / `src/auth/jwt_utils.cpp` | JWT generation and validation |
| `include/factory.hpp` / `src/patterns/factory.cpp` | Command factory (backward compat) |
| `CMakeLists.txt` | Build configuration with OpenSSL/Beast |
| `docker-compose.yml` | PostgreSQL database setup |
| `migrations/001_init.sql` | Database schema |

---

## Development Notes

### JWT Token Notes

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

## Legacy: How to run (TCP Protocol - Deprecated)

### 1. Start PostgreSQL in Docker
Run PostgreSQL container:

```bash
docker run --name db_service -e POSTGRES_PASSWORD=secret -p 5432:5432 -d postgres

If container already exists but is stopped:

```bash

docker start db_service
```

### 2. Create database schema
Run SQL manually inside container or use migration file.

### Example manual schema creation:

```bash

docker exec -it db_service psql -U postgres -d postgres
```

### Inside PostgreSQL console run:

```sql

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

\q
```

### 3. Build project
```bash

mkdir build
cd build
cmake ..
cmake --build .
```

### 4. Run server
```bash

./server
```

### 5. Run client
Open another terminal, go to build and run:

```bash

./client
```

### 6. Run tests

```bash

cd build

./tests
