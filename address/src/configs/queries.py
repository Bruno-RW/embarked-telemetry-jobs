QUERY_SELECT_LOCATION = """
    SELECT *
    FROM location;
"""

QUERY_CREATE_ADDRESS ="""
    CREATE TABLE address (
        id UUID PRIMARY KEY,
        country VARCHAR(255) NULL,
        country_code CHAR(2) NULL,
        state VARCHAR(255) NULL,
        region VARCHAR(255) NULL,
        city VARCHAR(255) NULL,
        postcode VARCHAR(255) NULL,
        road VARCHAR(255) NULL,
        house_number INT NULL
    );
"""

QUERY_INSERT_ADDRESS = """
    INSERT INTO address (id, country, country_code, state, region, city, postcode, road, house_number)
    VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s);
"""