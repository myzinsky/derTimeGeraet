#include "dertimegeraet.h"
#include "ui_dertimegeraet.h"

derTimeGeraet::derTimeGeraet(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::derTimeGeraet)
{
    ui->setupUi(this);
}

derTimeGeraet::~derTimeGeraet()
{
    delete ui;
}
