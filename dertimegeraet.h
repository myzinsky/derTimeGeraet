#ifndef DERTIMEGERAET_H
#define DERTIMEGERAET_H

#include <QCloseEvent>
#include <QFileSystemModel>
#include <QStringListModel>
#include <QLinkedList>
#include <QListWidgetItem>
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

    void on_pushButtonExeptionsRemove_clicked();

    void on_borgFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void on_pruneFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void on_pushButtonPassword_clicked();

    void on_pushButtonPruning_clicked();

    void on_listWidget_itemClicked(QListWidgetItem *item);

private:
    Ui::derTimeGeraet *ui;
    QFileSystemModel *dirModel;
    QSystemTrayIcon *trayIcon;
    QAction *quitAction;
    QAction *restoreAction;
    QTimer *timer;
    std::vector<QIcon> trayFrames;
    int trayIconNumber;
    QProcess *pBorg;
    QProcess *pPrune;
    QString time;
    QString borg;

    void loadBackupList();
    void mount(QString key);
    void umount();
    void loadIgnoreList();
    void saveIgnoreList();
    void locateBorg();
    void prune();

    // Settings:
    QString settingsFile;
};

#endif // DERTIMEGERAET_H
