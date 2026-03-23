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

-- Создадим один тестовый дом вручную, чтобы в него можно было добавлять квартиры!
INSERT INTO houses (address, build_year) VALUES ('ул. Пушкина, д. 1', 2026);