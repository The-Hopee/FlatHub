# Руководство по тестированию FlatHub

## Содержание
1. [Запуск unit тестов](#запуск-unit-тестов)
2. [Запуск приложения](#запуск-приложения)
3. [Тестирование API через curl](#тестирование-api-через-curl)
4. [Сценарии тестирования](#сценарии-тестирования)

---

## Запуск unit тестов

### Предварительные требования
```bash
# 1. Убедиться что PostgreSQL запущен в Docker
docker-compose up -d

# 2. Переходим в папку build
cd build

# 3. Пересоздаём конфигурацию CMake если нужна
cmake ..

# 4. Собираем проект
cmake --build .
```

### Запуск всех тестов
```bash
# Запуск всех юнит-тестов
./tests

# Или через ctest
ctest --output-on-failure

# Или через ctest с подробным выводом
ctest -V
```

**Ожидаемый результат:**
```
[==========] Running 25 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 25 tests from RepositoryTest, JWTUtilsTest
[ RUN      ] RepositoryTest.SaveUserAndFindByLogin
[       OK ] RepositoryTest.SaveUserAndFindByLogin (5 ms)
[ RUN      ] RepositoryTest.FindUnknownUserReturnsNullopt
[       OK ] RepositoryTest.FindUnknownUserReturnsNullopt (2 ms)
...
[==========] 25 tests passed (185 ms)
```

### Запуск конкретного теста
```bash
# Запуск только тестов JWTUtils
./tests --gtest_filter=JWTUtilsTest.*

# Запуск только теста валидации токена
./tests --gtest_filter=JWTUtilsTest.ValidateTokenAcceptsValidToken
```

---

## Запуск приложения

### 1. Подготовка

**Терминал 1: Запуск PostgreSQL (если не запущен)**
```bash
docker-compose up -d
```

**Ожидаемый результат:**
```
Creating flathub_db_service ... done
```

**Проверка что БД запущена:**
```bash
docker ps | grep db_service
```

**Ожидаемый результат:**
```
f6a2b1c3d4e5   postgres:15   "docker-entrypoint..." 2 days ago   Up 5 hours   0.0.0.0:5432->5432/tcp   flathub_db_service
```

### 2. Сборка приложения

**Терминал 1 (или новое окно):**
```bash
cd build
cmake ..
cmake --build .
```

**Ожидаемый результат:**
```
[100%] Linking CXX executable server
[100%] Built target server
```

### 3. Запуск сервера

**Терминал 2:**
```bash
cd build
./server
```

**Ожидаемый результат:**
```
[2025-01-15 14:32:45] INFO | MAIN_TRY | Starting HTTP server with JWT authentication...
[2025-01-15 14:32:45] INFO | HTTP_SERVER | ✓ HTTP сервер инициализирован на порту 8080
[2025-01-15 14:32:45] INFO | MAIN_TRY | HTTP Server started on port 8080. Listening for requests...
[2025-01-15 14:32:45] INFO | MAIN_TRY | Available endpoints: /api/login, /api/register, /api/flat, /api/flats/{id}, /api/house
```

**Сервер готов к приёму запросов!**

---

## Тестирование API через curl

### Переменные для удобства

**Терминал 3 (для тестирования):**
```bash
# Установим переменные окружения для удобства
BASE_URL="http://localhost:8080"
USER_TOKEN=""
MOD_TOKEN=""
```

---

## Сценарии тестирования

### Сценарий 1: Регистрация пользователя (успешно)

**Что тестируем:** Создание нового пользователя с ролью "user"

**Команда:**
```bash
curl -X POST $BASE_URL/api/register \
  -H "Content-Type: application/json" \
  -d '{"login":"alice","password":"pass123","role":"user"}'
```

**Ожидаемый результат (статус 201):**
```json
{
  "message": "Пользователь успешно зарегистрирован"
}
```

**Лог сервера:**
```
[2025-01-15 14:33:10] INFO | HTTP_SESSION | → Получен запрос: POST /api/register
[2025-01-15 14:33:10] INFO | DB_MANAGER | ✓ Пользователь успешно сохранён: alice
```

---

### Сценарий 2: Регистрация пользователя (дублирование)

**Что тестируем:** Попытка создать пользователя с существующим логином

**Команда:**
```bash
curl -X POST $BASE_URL/api/register \
  -H "Content-Type: application/json" \
  -d '{"login":"alice","password":"different","role":"user"}'
```

**Ожидаемый результат (статус 409):**
```json
{
  "error": "Ошибка: пользователь с логином 'alice' уже существует"
}
```

**Лог сервера:**
```
[2025-01-15 14:33:15] ERROR | DB_MANAGER | ✗ Ошибка при сохранении пользователя: уже существует
```

---

### Сценарий 3: Логин (успешно)

**Что тестируем:** Получение JWT токена при аутентификации

**Команда:**
```bash
curl -X POST $BASE_URL/api/login \
  -H "Content-Type: application/json" \
  -d '{"login":"alice","password":"pass123"}'
```

**Ожидаемый результат (статус 200):**
```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxLCJsb2dpbiI6ImFsaWNlIiwicm9sZSI6InVzZXIiLCJleHAiOjE3Mzc0NjI3OTV9.kX2f...",
  "role": "user",
  "message": "Успешная аутентификация"
}
```

**Сохраняем токен в переменную:**
```bash
USER_TOKEN="eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxLCJsb2dpbiI6ImFsaWNlIiwicm9sZSI6InVzZXIiLCJleHAiOjE3Mzc0NjI3OTU9.kX2f..."
```

**Лог сервера:**
```
[2025-01-15 14:33:20] INFO | HTTP_SESSION | → Получен запрос: POST /api/login
[2025-01-15 14:33:20] INFO | JWT | ✓ JWT токен сгенерирован для пользователя: alice
```

---

### Сценарий 4: Логин (неверный пароль)

**Что тестируем:** Отказ при неверном пароле

**Команда:**
```bash
curl -X POST $BASE_URL/api/login \
  -H "Content-Type: application/json" \
  -d '{"login":"alice","password":"wrongpassword"}'
```

**Ожидаемый результат (статус 401):**
```json
{
  "error": "Ошибка: неверный логин или пароль"
}
```

**Лог сервера:**
```
[2025-01-15 14:33:25] WARN | AUTH | ✗ Неверный пароль для пользователя: alice
```

---

### Сценарий 5: Логин (пользователь не найден)

**Что тестируем:** Отказ при несуществующем пользователе

**Команда:**
```bash
curl -X POST $BASE_URL/api/login \
  -H "Content-Type: application/json" \
  -d '{"login":"nonexistent","password":"anything"}'
```

**Ожидаемый результат (статус 404):**
```json
{
  "error": "Ошибка: пользователь не найден"
}
```

---

### Сценарий 6: Регистрация модератора

**Что тестируем:** Создание пользователя с ролью "moderator"

**Команда:**
```bash
curl -X POST $BASE_URL/api/register \
  -H "Content-Type: application/json" \
  -d '{"login":"bob","password":"modpass","role":"moderator"}'
```

**Ожидаемый результат (статус 201):**
```json
{
  "message": "Пользователь успешно зарегистрирован"
}
```

**Логиним модератора и сохраняем токен:**
```bash
curl -X POST $BASE_URL/api/login \
  -H "Content-Type: application/json" \
  -d '{"login":"bob","password":"modpass"}'
```

**Результат:**
```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoyLCJsb2dpbiI6ImJvYiIsInJvbGUiOiJtb2RlcmF0b3IiLCJleHAiOjE3Mzc0NjI3OTU9.abcd...",
  "role": "moderator",
  "message": "Успешная аутентификация"
}
```

**Сохраняем в переменную:**
```bash
MOD_TOKEN="eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoyLCJsb2dpbiI6ImJvYiIsInJvbGUiOiJtb2RlcmF0b3IiLCJleHAiOjE3Mzc0NjI3OTU9.abcd..."
```

---

### Сценарий 7: Создание дома (только модератор)

**Что тестируем:** Создание нового дома с данными

**Команда:**
```bash
curl -X POST $BASE_URL/api/house \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $MOD_TOKEN" \
  -d '{
    "address":"Тверская улица, 15",
    "build_year":2023,
    "developer":"ПИК"
  }'
```

**Ожидаемый результат (статус 201):**
```json
{
  "message": "Дом успешно создан",
  "house_id": 1
}
```

**Лог сервера:**
```
[2025-01-15 14:33:30] INFO | HTTP_SESSION | → Получен запрос: POST /api/house
[2025-01-15 14:33:30] INFO | AUTH | ✓ Токен валидирован: bob (moderator)
[2025-01-15 14:33:30] INFO | DB | ✓ Дом успешно сохранён: ID=1
```

**Сохраняем ID дома:**
```bash
HOUSE_ID=1
```

---

### Сценарий 8: Попытка создания дома (обычный пользователь - ошибка)

**Что тестируем:** Отказ доступа для обычного пользователя

**Команда:**
```bash
curl -X POST $BASE_URL/api/house \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $USER_TOKEN" \
  -d '{
    "address":"Новая улица, 20",
    "build_year":2024,
    "developer":"Лукойл"
  }'
```

**Ожидаемый результат (статус 403):**
```json
{
  "error": "Ошибка: недостаточно прав. Требуется роль: moderator"
}
```

**Лог сервера:**
```
[2025-01-15 14:33:35] WARN | AUTH | ✗ Доступ запрещён: требуется роль moderator, текущая роль: user
```

---

### Сценарий 9: Создание квартиры (с валидным токеном)

**Что тестируем:** Создание новой квартиры в доме

**Команда:**
```bash
curl -X POST $BASE_URL/api/flat \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $USER_TOKEN" \
  -d '{
    "house_id":1,
    "flat_number":"101",
    "rooms":2,
    "price":5000000
  }'
```

**Ожидаемый результат (статус 201):**
```json
{
  "message": "Квартира успешно создана",
  "flat_id": 1,
  "status": "created"
}
```

**Лог сервера:**
```
[2025-01-15 14:33:40] INFO | HTTP_SESSION | → Получен запрос: POST /api/flat
[2025-01-15 14:33:40] INFO | AUTH | ✓ Токен валидирован: alice (user)
[2025-01-15 14:33:40] INFO | DB | ✓ Квартира успешно сохранена: ID=1
```

**Сохраняем ID квартиры:**
```bash
FLAT_ID=1
```

---

### Сценарий 10: Получение квартир дома (обычный пользователь видит только одобренные)

**Что тестируем:** Фильтрация квартир по статусу для обычного пользователя

**Команда:**
```bash
curl -X GET "$BASE_URL/api/flats/1" \
  -H "Authorization: Bearer $USER_TOKEN"
```

**Ожидаемый результат (статус 200):**
```json
{
  "flats": [],
  "message": "Квартиры дома ID=1 (статус: одобренные)"
}
```

**Пояснение:** Квартира с ID=1 имеет статус "created", поэтому обычный пользователь её не видит. Видны только квартиры со статусом "approved".

---

### Сценарий 11: Взятие квартиры на модерацию (модератор)

**Что тестируем:** Модератор берёт квартиру на модерацию

**Команда:**
```bash
curl -X POST "$BASE_URL/api/flat/take" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $MOD_TOKEN" \
  -d '{"flat_id":1}'
```

**Ожидаемый результат (статус 200):**
```json
{
  "message": "Квартира успешно взята на модерацию",
  "new_status": "on_moderation"
}
```

**Лог сервера:**
```
[2025-01-15 14:33:45] INFO | HTTP_SESSION | → Получен запрос: POST /api/flat/take
[2025-01-15 14:33:45] INFO | AUTH | ✓ Токен валидирован: bob (moderator)
[2025-01-15 14:33:45] INFO | DB | ✓ Статус квартиры изменён: created → on_moderation
```

---

### Сценарий 12: Обновление статуса квартиры (одобрение)

**Что тестируем:** Модератор одобряет квартиру

**Команда:**
```bash
curl -X PUT "$BASE_URL/api/flat/status" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $MOD_TOKEN" \
  -d '{"flat_id":1,"new_status":"approved"}'
```

**Ожидаемый результат (статус 200):**
```json
{
  "message": "Статус квартиры успешно обновлён",
  "flat_id": 1,
  "new_status": "approved"
}
```

**Лог сервера:**
```
[2025-01-15 14:33:50] INFO | HTTP_SESSION | → Получен запрос: PUT /api/flat/status
[2025-01-15 14:33:50] INFO | AUTH | ✓ Токен валидирован: bob (moderator)
[2025-01-15 14:33:50] INFO | DB | ✓ Статус квартиры изменён: on_moderation → approved
```

---

### Сценарий 13: Получение одобренной квартиры (обычный пользователь)

**Что тестируем:** Обычный пользователь видит одобренные квартиры

**Команда:**
```bash
curl -X GET "$BASE_URL/api/flats/1" \
  -H "Authorization: Bearer $USER_TOKEN"
```

**Ожидаемый результат (статус 200):**
```json
{
  "flats": [
    {
      "id": 1,
      "flat_number": "101",
      "price": 5000000,
      "rooms": 2,
      "status": "approved"
    }
  ],
  "message": "Квартиры дома ID=1 (статус: одобренные)"
}
```

---

### Сценарий 14: Модератор видит все квартиры

**Что тестируем:** Модератор видит квартиры со всеми статусами

**Команда:**
```bash
curl -X GET "$BASE_URL/api/flats/1" \
  -H "Authorization: Bearer $MOD_TOKEN"
```

**Ожидаемый результат (статус 200):**
```json
{
  "flats": [
    {
      "id": 1,
      "flat_number": "101",
      "price": 5000000,
      "rooms": 2,
      "status": "approved"
    }
  ],
  "message": "Квартиры дома ID=1 (статус: все)"
}
```

---

### Сценарий 15: Попытка использования истёкшего токена

**Что тестируем:** Отказ при истёкшем токене

**Сначала создаём токен на очень короткое время:**
```bash
# Это требует прямого взаимодействия с кодом - пропускаем для демо
# или используем старый токен из предыдущего сеанса
```

**Команда (с истёкшим токеном):**
```bash
curl -X GET "$BASE_URL/api/flats/1" \
  -H "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxLCJsb2dpbiI6ImFsaWNlIiwicm9sZSI6InVzZXIiLCJleHAiOjE3MzcyNDAwMDB9.old..."
```

**Ожидаемый результат (статус 401):**
```json
{
  "error": "Ошибка: токен истёк или невалиден"
}
```

---

### Сценарий 16: Запрос без токена (ошибка)

**Что тестируем:** Отказ при отсутствии токена для защищённого эндпоинта

**Команда:**
```bash
curl -X GET "$BASE_URL/api/flats/1"
```

**Ожидаемый результат (статус 401):**
```json
{
  "error": "Ошибка: отсутствует токен авторизации"
}
```

---

### Сценарий 17: Попытка взять уже взятую квартиру (ошибка)

**Что тестируем:** Невозможно взять квартиру со статусом отличным от "created"

**Сначала возьмём квартиру:**
```bash
curl -X POST "$BASE_URL/api/flat/take" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $MOD_TOKEN" \
  -d '{"flat_id":1}'
```

**Результат:** Успех

**Попытаемся взять её ещё раз:**
```bash
curl -X POST "$BASE_URL/api/flat/take" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $MOD_TOKEN" \
  -d '{"flat_id":1}'
```

**Ожидаемый результат (статус 409):**
```json
{
  "error": "Ошибка: квартира не в статусе 'created', невозможно взять на модерацию"
}
```

---

### Сценарий 18: Отказ в одобрении квартиры

**Что тестируем:** Модератор может отклонить квартиру

**Команда:**
```bash
curl -X PUT "$BASE_URL/api/flat/status" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $MOD_TOKEN" \
  -d '{"flat_id":1,"new_status":"declined"}'
```

**Ожидаемый результат (статус 200):**
```json
{
  "message": "Статус квартиры успешно обновлён",
  "flat_id": 1,
  "new_status": "declined"
}
```

---

### Сценарий 19: Неверная структура JSON

**Что тестируем:** Обработка некорректного JSON

**Команда:**
```bash
curl -X POST $BASE_URL/api/login \
  -H "Content-Type: application/json" \
  -d 'это не JSON'
```

**Ожидаемый результат (статус 400):**
```json
{
  "error": "Ошибка: некорректный JSON"
}
```

---

### Сценарий 20: Полная история: от регистрации до одобрения

**Этот сценарий объединяет все предыдущие:**

```bash
# 1. Регистрируем обычного пользователя
curl -X POST $BASE_URL/api/register \
  -H "Content-Type: application/json" \
  -d '{"login":"user1","password":"pass123","role":"user"}'

# 2. Регистрируем модератора
curl -X POST $BASE_URL/api/register \
  -H "Content-Type: application/json" \
  -d '{"login":"moderator1","password":"modpass123","role":"moderator"}'

# 3. Логим пользователя и сохраняем токен
USER_TOKEN=$(curl -s -X POST $BASE_URL/api/login \
  -H "Content-Type: application/json" \
  -d '{"login":"user1","password":"pass123"}' | jq -r '.token')

# 4. Логим модератора и сохраняем токен
MOD_TOKEN=$(curl -s -X POST $BASE_URL/api/login \
  -H "Content-Type: application/json" \
  -d '{"login":"moderator1","password":"modpass123"}' | jq -r '.token')

# 5. Модератор создаёт дом
HOUSE_ID=$(curl -s -X POST $BASE_URL/api/house \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $MOD_TOKEN" \
  -d '{"address":"Проспект Вернадского, 42","build_year":2024,"developer":"Девелопер"}' \
  | jq -r '.house_id')

# 6. Пользователь создаёт квартиру
FLAT_ID=$(curl -s -X POST $BASE_URL/api/flat \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $USER_TOKEN" \
  -d "{\"house_id\":$HOUSE_ID,\"flat_number\":\"501\",\"rooms\":3,\"price\":8500000}" \
  | jq -r '.flat_id')

# 7. Модератор берёт квартиру на модерацию
curl -s -X POST "$BASE_URL/api/flat/take" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $MOD_TOKEN" \
  -d "{\"flat_id\":$FLAT_ID}"

# 8. Модератор одобряет квартиру
curl -s -X PUT "$BASE_URL/api/flat/status" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $MOD_TOKEN" \
  -d "{\"flat_id\":$FLAT_ID,\"new_status\":\"approved\"}"

# 9. Пользователь видит одобренную квартиру
curl -s -X GET "$BASE_URL/api/flats/$HOUSE_ID" \
  -H "Authorization: Bearer $USER_TOKEN" | jq .

echo "✓ Полный цикл завершён успешно!"
```

---

## Проверка логов приложения

**Просмотр логов в реальном времени:**
```bash
tail -f logs/app.log
```

**Просмотр последних 50 строк лога:**
```bash
tail -50 logs/app.log
```

**Поиск в логах:**
```bash
# Все ошибки
grep "ERROR" logs/app.log

# Все успешные операции
grep "✓\|SUCCESS" logs/app.log

# Операции конкретного пользователя
grep "alice" logs/app.log
```

---

## Советы для тестирования

1. **Используйте `jq` для красивого вывода JSON:**
   ```bash
   curl -s $BASE_URL/api/register ... | jq .
   ```

2. **Сохраняйте значения в переменные:**
   ```bash
   TOKEN=$(curl -s ... | jq -r '.token')
   ```

3. **Проверяйте статус ответа:**
   ```bash
   curl -w "\nСтатус: %{http_code}\n" ...
   ```

4. **Используйте `-v` для подробного вывода:**
   ```bash
   curl -v -X POST ...
   ```

5. **Используйте `-i` для просмотра заголовков:**
   ```bash
   curl -i -X POST ...
   ```

---

## Останов приложения

**Остановить сервер (Ctrl+C в терминале):**
```
^C
[2025-01-15 14:35:00] INFO | HTTP_SERVER | ↓ Остановка HTTP сервера...
```

**Остановить PostgreSQL:**
```bash
docker-compose down
```

**Или остановить только контейнер:**
```bash
docker stop db_service
```

---

## Решение проблем

### Ошибка: "Connection refused" на порту 8080
- Проверьте что сервер запущен: `ps aux | grep server`
- Проверьте логи: `tail logs/app.log`
- Перезагрузите сервер

### Ошибка: "FATAL: remaining connection slots are reserved"
- Слишком много подключений к БД
- Перезагрузите Docker: `docker-compose restart`

### Ошибка: "Password authentication failed"
- Проверьте строку подключения в коде
- Проверьте что PostgreSQL запущен

---

**Последнее обновление:** 20 апреля 2026 г.
