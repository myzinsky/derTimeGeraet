#ifndef DERTIMEGERAET_H
#define DERTIMEGERAET_H

#include <QMainWindow>

namespace Ui {
class derTimeGeraet;
}

class derTimeGeraet : public QMainWindow
{
    Q_OBJECT

public:
    explicit derTimeGeraet(QWidget *parent = 0);
    ~derTimeGeraet();

private:
    Ui::derTimeGeraet *ui;
};

#endif // DERTIMEGERAET_H
