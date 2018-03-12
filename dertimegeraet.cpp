#include "dertimegeraet.h"
#include "ui_dertimegeraet.h"
#include <QFileDialog>
#include <QFileSystemModel>
#include <QDebug>
#include <QDesktopServices>
#include <QDateTime>
#include <QProcess>
#include <QCloseEvent>
#include <QMessageBox>
#include <QTimer>
#include <QMenu>


derTimeGeraet::derTimeGeraet(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::derTimeGeraet)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(1);

    // Setup Settings:
    settingsFile = QApplication::applicationDirPath() + "/settings.ini";
    bool settingsAvailable = loadSettings();

    // Setup Dir Model:
    QString dir = ui->lineEditDest->text();
    if(settingsAvailable == true && QDir(dir).exists())
    {
        dirModel = new QFileSystemModel(this);
        dirModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
        dirModel->setRootPath("/");
        ui->listView->setModel(dirModel);
        ui->listView->setRootIndex(dirModel->index(dir));
        ui->tabWidget->setCurrentIndex(0);
    }
    else
    {
        ui->listView->setEnabled(false);
        ui->treeView->setEnabled(false);
        ui->pushButtonStart->setEnabled(false);
    }

    // Load Ignore List:
    loadIgnoreList();

    // Setup Tray:
    setupTrayIcon();

    // Prevent close from closing:
    this->setWindowFlags(Qt::Tool);
    QApplication::setQuitOnLastWindowClosed(false);
}

derTimeGeraet::~derTimeGeraet()
{
    delete ui;
}

void derTimeGeraet::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible())
    {
        return;
    }
#endif
    if (trayIcon->isVisible())
    {
        QSettings settings(settingsFile, QSettings::NativeFormat);
        if(settings.value("closeHint", "") == 0)
        {
            QMessageBox::information(this, tr("Systray"),
                tr("The program will keep running in the "
                   "system tray. To terminate the program, "
                   "choose <b>Quit</b> in the context menu "
                   "of the system tray entry."));
            settings.value("closeHint", 1);
        }
        hide();
        event->ignore();
    }
}

void derTimeGeraet::setupTrayIcon()
{
    trayFrames.push_back(QIcon(":/img/gear-white.svg"));
    trayFrames.push_back(QIcon(":/img/gear-white2.svg"));

    trayIcon = new QSystemTrayIcon(trayFrames.at(0), this);
    trayIconNumber = 0;

    restoreAction = new QAction(tr("&Show"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    QMenu *tray_icon_menu = new QMenu;
    tray_icon_menu->addAction(quitAction);
    tray_icon_menu->addAction(restoreAction);
    trayIcon->setContextMenu( tray_icon_menu );
    trayIcon->show();

    timer = new QTimer(this);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTrayIcon()));
}

void derTimeGeraet::updateTrayIcon()
{
    if(trayIconNumber == 0)
    {
        trayIconNumber = 1;
        trayIcon->setIcon(trayFrames.at(trayIconNumber));
    }
    else
    {
        trayIconNumber = 0;
        trayIcon->setIcon(trayFrames.at(trayIconNumber));
    }
}

void derTimeGeraet::setTrayIcon(bool animated)
{
    if(animated == true)
    {
        timer->start(500); // 500 ms
    }
    else
    {
        timer->stop();
    }
}


bool derTimeGeraet::loadSettings()
{
    if (QFile(settingsFile).exists())
    {
        QSettings settings(settingsFile, QSettings::NativeFormat);
        QString source = settings.value("source", "").toString();
        QString dest = settings.value("dest", "").toString();

        ui->lineEditDest->setText(dest);
        ui->lineEditSource->setText(source);

        ui->tab1->setEnabled(true);
        ui->pushButtonStart->setEnabled(true);
        return true;
    }
    else
    {
        ui->tab1->setEnabled(false);
        return false;
    }
}

void derTimeGeraet::saveSettings()
{
    QSettings settings(settingsFile, QSettings::NativeFormat);

    if(!ui->lineEditDest->text().isEmpty())
    {
         QString dest = ui->lineEditDest->text();
         settings.setValue("dest", dest);
    }

    if(!ui->lineEditSource->text().isEmpty())
    {
         QString source = ui->lineEditSource->text();
         settings.setValue("source", source);
    }

    if(!ui->lineEditDest->text().isEmpty() && !ui->lineEditSource->text().isEmpty())
    {
        ui->listView->setEnabled(false);
        ui->treeView->setEnabled(false);
        ui->pushButtonStart->setEnabled(false);
    }
}

void derTimeGeraet::on_pushButtonSource_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                tr("Select Directory"),
                                                "/",
                                                QFileDialog::ShowDirsOnly);
    ui->lineEditSource->setText(dir);
    saveSettings();
}

void derTimeGeraet::on_pushButtonDest_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                tr("Select Directory"),
                                                "/",
                                                QFileDialog::ShowDirsOnly);
    ui->lineEditDest->setText(dir);
    saveSettings();
}

void derTimeGeraet::on_listView_clicked(const QModelIndex &index)
{
    QString dir = ui->listView->model()->data(index, QFileSystemModel::FilePathRole).toString();

    if(QDir(dir).exists())
    {
        dirModel = new QFileSystemModel(this);
        dirModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
        dirModel->setRootPath("/");
        ui->treeView->setModel(dirModel);
        ui->treeView->setRootIndex(dirModel->index(dir));
        ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    }
}

void derTimeGeraet::on_treeView_doubleClicked(const QModelIndex &index)
{
    QString file = ui->treeView->model()->data(index, QFileSystemModel::FilePathRole).toString();
    QDesktopServices::openUrl(QUrl("file://" + file, QUrl::TolerantMode));
}

void derTimeGeraet::on_pushButtonStart_clicked()
{
    QString dest = ui->lineEditDest->text();
    QString source = ui->lineEditSource->text();

    if(QDir(dest).exists() && QDir(source).exists())
    {
        QString time = QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
        QStringList cmdRsync;
        cmdRsync.append("-a");
        cmdRsync.append("-h");
        cmdRsync.append("-P");

        for(int i = 0; i < ui->listWidgetExeptions->count(); i++)
        {
            QString ignore = ui->listWidgetExeptions->item(i)->text();
            ignore.replace(source, "");
            cmdRsync.append("--exclude="+ignore);
        }
        cmdRsync.append("--link-dest=" + dest + "/current");
        cmdRsync.append(source+"/");
        cmdRsync.append(dest + "/" + time);

        qDebug() << cmdRsync;

        QStringList cmdRm;
        cmdRm.append("-f");
        cmdRm.append(dest + "/current");

        QStringList cmdLn;
        cmdLn.append("-s");
        cmdLn.append(dest + "/" + time);
        cmdLn.append(dest + "/current");

        ui->pushButtonStart->setText("Preparing Backup (This may take a while)");
        ui->pushButtonStart->setDisabled(true);
        setTrayIcon(true);

        pRsync = new QProcess(this);
        connect(pRsync, SIGNAL(readyReadStandardOutput()), this, SLOT(rsyncOutput()));
        connect(pRsync, SIGNAL(readyReadStandardError()),  this, SLOT(rsyncOutput()));
        pRsync->start("/usr/bin/rsync", cmdRsync);
        pRsync->waitForFinished(-1);

        delete pRsync;

        QProcess pRm;
        pRm.start("/bin/rm", cmdRm);
        pRm.waitForFinished(-1);

        QProcess pLn;
        pLn.start("/bin/ln", QStringList() << cmdLn);
        pLn.waitForFinished(-1);

        ui->pushButtonStart->setEnabled(true);
        ui->pushButtonStart->setText("Start Backup");
        setTrayIcon(false);
    }
}

void derTimeGeraet::on_pushButtonExeptionsAdd_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                tr("Select Directory"),
                                                "/",
                                                QFileDialog::ShowDirsOnly);

    // TODO: check if item already exists ...
    ui->listWidgetExeptions->addItem(dir);
    saveIgnoreList();
}

void derTimeGeraet::loadIgnoreList()
{
    QSettings settings(settingsFile, QSettings::NativeFormat);
    int size = settings.beginReadArray("Ignores");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString ignore = settings.value("Ignore").toString();
        ui->listWidgetExeptions->addItem(ignore);
    }
    settings.endArray();
}

void derTimeGeraet::saveIgnoreList()
{
    QSettings settings(settingsFile, QSettings::NativeFormat);
    settings.beginWriteArray("Ignores");
    for(int i = 0; i < ui->listWidgetExeptions->count(); i++)
    {
        QString ignore = ui->listWidgetExeptions->item(i)->text();
        settings.setArrayIndex(i);
        settings.setValue("Ignore", ignore);
    }
    settings.endArray();
}

void derTimeGeraet::on_pushButtonExeptionsRemove_clicked()
{
   QList<QListWidgetItem*> items = ui->listWidgetExeptions->selectedItems();
   foreach(QListWidgetItem * item, items)
   {
       delete ui->listWidgetExeptions->takeItem(ui->listWidgetExeptions->row(item));
   }
   saveIgnoreList();
}

void derTimeGeraet::rsyncOutput()
{
    qDebug() << pRsync->readAllStandardOutput();
    qDebug() << pRsync->readAllStandardError();
}
