#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "../appwindow.h"

namespace Ui {
class Authentication;
}

class Authentication : public AppWindow
{
    Q_OBJECT

public:
    explicit Authentication(QWidget *parent = nullptr);
    ~Authentication() override;

private:
    Ui::Authentication *ui;
};

#endif // AUTHENTICATION_H
