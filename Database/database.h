#ifndef DATABASE_H
#define DATABASE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui {
class database;
}
QT_END_NAMESPACE

class Database : public QWidget
{
    Q_OBJECT

public:
    explicit Database(QWidget *parent = nullptr);
    virtual ~Database();

protected:
    QSqlDatabase db;
    QString loginVal;
    QString nickVal;
    QString passVal;

    bool connectToDatabase();
    bool initializeConnection();
};

#endif // DATABASE_H
