#include "authentication.h"
#include "ui_authentication.h"

Authentication::Authentication(QWidget *parent)
    : AppWindow(parent)
    , ui(new Ui::Authentication)
{
    ui->setupUi(this);

    setWindowTitle("Login");
}

Authentication::~Authentication()
{
    delete ui;
}
