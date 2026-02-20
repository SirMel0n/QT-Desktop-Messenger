#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "../Registration/registration.h" // includes registration class

namespace Ui {
class Authentication;
}

class Authentication : public Registration
{
    Q_OBJECT

public:
    explicit Authentication(QWidget *parent = nullptr);
    ~Authentication() override;

private:
    Ui::Authentication *ui;
};

#endif // AUTHENTICATION_H
