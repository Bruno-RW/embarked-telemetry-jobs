import os
from dotenv import load_dotenv

import psycopg2
from psycopg2 import OperationalError

class PostgreConnection:
    """
        PostgreConnection.py

        This module provides a singleton class `PostgreConnection` to manage a PostgreSQL database connection.
        It ensures that only one connection instance is created and reused throughout the application.

        Classes:
            PostgreConnection: A singleton class to handle PostgreSQL database connections.

        Methods:
            connect():
                Establishes a connection to the PostgreSQL database using credentials from environment variables.
            disconnect():
                Closes the database connection and cursor if they are open.
            getCursor():
                Returns the database cursor, establishing a connection if it doesn't already exist.
            getConnection():
                Returns the database connection, establishing it if it doesn't already exist.

        Usage:
            from database.PostgreConnection import PostgreConnection

            # Get the singleton instance
            db_connection = PostgreConnection()

            # Connect to the database
            db_connection.connect()

            # Get the cursor and execute a query
            cursor = db_connection.getCursor()
            cursor.execute("SELECT 1;")
            print(cursor.fetchone())

            # Disconnect from the database
            db_connection.disconnect()
    """

    #~ Class variables
    _instance = None

    def __new__(cls, *args, **kwargs):
        """
            Ensures that only one instance of the PostgreConnection class is created (singleton pattern).

            Args:
                *args: Variable length argument list.
                **kwargs: Arbitrary keyword arguments.

            Returns:
                PostgreConnection: The singleton instance of the PostgreConnection class.
        """
        
        if not cls._instance:
            cls._instance = super(PostgreConnection, cls).__new__(cls, *args, **kwargs)

        return cls._instance

    def __init__(self):
        """
            Initializes the PostgreConnection instance.

            This method ensures that the environment variables are loaded and initializes
            the `_connection` and `_cursor` attributes to `None` if they are not already set.
        """

        if not hasattr(self, "_connection"):
            load_dotenv()

            self._connection = None
            self._cursor = None

    def connect(self):
        """
            Establishes a connection to the PostgreSQL database.

            The connection is created using credentials from environment variables:
            - POSTGRE_HOST
            - POSTGRE_PORT
            - POSTGRE_DATABASE
            - POSTGRE_USER
            - POSTGRE_PASSWORD

            Raises:
                OperationalError: If there is an error connecting to the database.
        """

        if not self._connection:
            try:
                self._connection = psycopg2.connect(
                    host     = os.getenv("POSTGRE_HOST"),
                    port     = os.getenv("POSTGRE_PORT"),
                    database = os.getenv("POSTGRE_DATABASE"),
                    user     = os.getenv("POSTGRE_USER"),
                    password = os.getenv("POSTGRE_PASSWORD"),
                )

                self._cursor = self._connection.cursor()
                print("Database connection established.")

            except OperationalError as e:
                print(f"Error connecting to the database: {e}")
                raise

    def disconnect(self):
        """
            Closes the database connection and cursor if they are open.

            This method ensures that the `_connection` and `_cursor` attributes are set to `None`
            after closing the connection and cursor.
        """
        
        try:

            if self._cursor:
                self._cursor.close()
                self._cursor = None
                print("PostgreSQL cursor closed.")

            if self._connection:
                self._connection.close()
                self._connection = None
                print("PostgreSQL connection closed.")

            return True

        except Exception as e:
            print(f"Error disconnecting to PostgreSQL: {e}")
            return False

    def getCursor(self):
        """
            Returns the database cursor.

            If the connection is not already established, this method calls `connect()` to establish it.

            Returns:
                psycopg2.extensions.cursor: The database cursor.
        """

        try:
            if not self._connection: self.connect()
            return self._cursor

        except Exception as e:
            print(f"Error getting PostgreSQL cursor: {e}")
            return None

    def getConnection(self):
        """
            Returns the database connection.

            If the connection is not already established, this method calls `connect()` to establish it.

            Returns:
                psycopg2.extensions.connection: The database connection.
        """

        try:
            if not self._connection: self.connect()
            return self._connection
        
        except Exception as e:
            print(f"Error getting PostgreSQL connection: {e}")
            return None