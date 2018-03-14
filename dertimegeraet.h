#ifndef DERTIMEGERAET_H
#define DERTIMEGERAET_H

#include <QCloseEvent>
#include <QFileSystemModel>
#include <QLinkedList>
#include <QMainWindow>
#include <QProcess>
#include <QSettings>
#include <QSystemTrayIcon>

namespace Ui {
class derTimeGeraet;
}

class derTimeGeraet : public QMainWindow
{
    Q_OBJECT

public:
    explicit derTimeGeraet(QWidget *parent = 0);
    ~derTimeGeraet();
    bool loadSettings();
    void saveSettings();

private slots:
    void on_pushButtonSource_clicked();

    void on_pushButtonDest_clicked();

    void on_listView_clicked(const QModelIndex &index);

    void on_treeView_doubleClicked(const QModelIndex &index);

    void on_pushButtonStart_clicked();

    void closeEvent(QCloseEvent *event) override;

    void setTrayIcon(bool animated);

    void updateTrayIcon();

    void setupTrayIcon();

    void on_pushButtonExeptionsAdd_clicked();

    void loadIgnoreList();

    void saveIgnoreList();

    void on_pushButtonExeptionsRemove_clicked();

    void on_rsyncFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Ui::derTimeGeraet *ui;
    QFileSystemModel *dirModel;
    QSystemTrayIcon *trayIcon;
    QAction *quitAction;
    QAction *restoreAction;
    QTimer *timer;
    std::vector<QIcon> trayFrames;
    int trayIconNumber;
    QProcess *pRsync;
    QString time;

    // Settings:
    QString settingsFile;
};

#endif // DERTIMEGERAET_H
