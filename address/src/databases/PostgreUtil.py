from psycopg2.extras import RealDictCursor

from .PostgreConnection import PostgreConnection

class PostgreUtil:
    """
    PostgreUtil

    Utility class for performing PostgreSQL database operations, such as creating tables,
    inserting rows, and executing queries.

    Attributes:
        connection: The active PostgreSQL connection.
        cursor: The active PostgreSQL cursor.

    Methods:
        select(query, params=None, fetchType="all"):
            Executes a SELECT query and returns the results.
    """

    def __init__(self):
        """
        Initializes the PostgreUtil class.

        Automatically gets the singleton PostgreConnection and uses its connection.
        """

        self.connection = PostgreConnection().getConnection()

    def select(self, query, params=None, fetchType="all"):
        """Execute a SELECT query and return the results."""

        try:
            with self.connection.cursor(cursor_factory=RealDictCursor) as cursor:
                cursor.execute(query, params)

                if   fetchType == "one":  return cursor.fetchone()
                elif fetchType == "many": return cursor.fetchmany()
                elif fetchType == "all":  return cursor.fetchall()
                
                else: raise ValueError("Invalid fetchType. Use 'one', 'many', or 'all'.")
        
        except Exception as e:
            print(f"Error executing select query: {e}")
            raise

    def insert(self, query, params=None):
        try:
            with self.connection.cursor() as cursor:
                cursor.execute(query, params)
                self.connection.commit()
            
        except Exception as e:
            print(f"Error inserting location data: {e}")
            raise

    def batchInsert(self, query, rows):
        try:
            with self.connection.cursor() as cursor:
                cursor.executemany(query, rows)
                self.connection.commit()

        except Exception as e:
            print(f"Error inserting multiple rows: {e}")
            raise
    
    def createTable(self, query):
        try:
            with self.connection.cursor() as cursor:
                cursor.execute(query)
                self.connection.commit()
            
        except Exception as e:
            print(f"Error creating table: {e}")
            raise