#include "database.h"

Database::Database(QWidget *parent)
    : QWidget(parent)
    , passVal(" ")
    , nickVal(" ")
    , loginVal(" ")
{
}

Database::~Database()
{
    // Close database connection
    if (db.isOpen()) {
        db.close();
    }
}

bool Database::connectToDatabase()
{
    // Check if connection already exists and is open
    if (db.isOpen()) {
        qDebug() << "Database already connected";
        return true;
    }

    // Check if database object has been initialized but not opened
    if (!db.isValid()) {
        // Add ODBC driver
        db = QSqlDatabase::addDatabase("QODBC");

        // Set connection string
        db.setDatabaseName("DRIVER={MySQL ODBC 9.6 Unicode Driver};"
                           "SERVER=localhost;"
                           "PORT=3306;"
                           "DATABASE=qtappdb;"
                           "USER=qtapp;"
                           "PASSWORD=123456789;"
                           "OPTION=3;");
    }

    // Open the connection
    if (!db.open()) {
        // If connection fails, show error message
        QMessageBox::critical(this, "Database Error",
                              "Failed to connect to database:\n" +
                                  db.lastError().text());
        return false;
    }

    qDebug() << "Database connected successfully";
    return true;
}

bool Database::initializeConnection()
{
    return connectToDatabase();
}
