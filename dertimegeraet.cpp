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

    // Locate Borg:
    locateBorg();

    // Load List of Backups:
    loadBackupList();

    // Load Ignore List:
    loadIgnoreList();

    // Setup Tray:
    setupTrayIcon();

    // Prevent close from closing:
    this->setWindowFlags(Qt::Widget);
    QApplication::setQuitOnLastWindowClosed(false);
}

void derTimeGeraet::loadBackupList()
{
    // Setup Settings:
    settingsFile = QApplication::applicationDirPath() + "/settings.ini";
    bool settingsAvailable = loadSettings();

    // Setup Dir Model:
    QString dir = ui->lineEditDest->text();
    if(settingsAvailable == true && QDir(dir).exists())
    {
        ui->listWidget->clear();

        QSettings settings(settingsFile, QSettings::NativeFormat);
        QString dest = settings.value("dest", "").toString();
        QString pass = settings.value("pass", "").toString();

        QStringList cmdList;
        cmdList.append("list");
        cmdList.append(dest);
        QProcess pList;

        qDebug() << cmdList;

        // Add pass as environment variable:
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("BORG_PASSPHRASE", pass);
        pList.setProcessEnvironment(env);

        pList.start(borg, cmdList);
        pList.waitForFinished(-1);
        QString listString = pList.readAllStandardOutput();

        QStringList list = listString.split('\n');

        // Create model
        for(int i = 0; i < list.size(); i++)
        {
            qDebug() << list.at(i);
            QRegExp rx("([^\\s]+)\\s+(.+)\\s+\\[(.+)\\]");
            QString name = "";
            QString key = "";
            bool match = rx.exactMatch(list.at(i));
            if (match) {
                name = rx.cap(2);
                key = rx.cap(1);
            }

            auto *item = new QListWidgetItem(name);
            QVariant v;
            v.setValue(key);
            item->setData(Qt::UserRole, v);
            ui->listWidget->addItem(item);
        }

        ui->tabWidget->setCurrentIndex(0);
    }
    else
    {
        ui->listWidget->setEnabled(false);
        ui->treeView->setEnabled(false);
        ui->pushButtonStart->setEnabled(false);
    }
}

derTimeGeraet::~derTimeGeraet()
{
    qDebug() << "UMOUNT!";
    umount();
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
        QString source  = settings.value("source", "").toString();
        QString dest    = settings.value("dest", "").toString();
        QString pass    = settings.value("pass", "").toString();
        QString hourly  = settings.value("hourly", "").toString();
        QString daily   = settings.value("daily", "").toString();
        QString weekly  = settings.value("weekly", "").toString();
        QString monthly = settings.value("monthly", "").toString();
        QString yearly  = settings.value("yearly", "").toString();

        ui->lineEditDest->setText(dest);
        ui->lineEditSource->setText(source);
        ui->linePassword->setText(pass);
        ui->lineEditHourly->setText(hourly);
        ui->lineEditDaily->setText(daily);
        ui->lineEditWeekly->setText(weekly);
        ui->lineEditMonthly->setText(monthly);
        ui->lineEditYearly->setText(yearly);

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

    if(!ui->linePassword->text().isEmpty())
    {
         QString pass = ui->linePassword->text();
         settings.setValue("pass", pass);
    }

    if(!ui->lineEditHourly->text().isEmpty())
    {
         QString hourly = ui->lineEditHourly->text();
         settings.setValue("hourly", hourly);
    }

    if(!ui->lineEditDaily->text().isEmpty())
    {
         QString daily = ui->lineEditDaily->text();
         settings.setValue("daily", daily);
    }

    if(!ui->lineEditWeekly->text().isEmpty())
    {
         QString weekly = ui->lineEditWeekly->text();
         settings.setValue("weekly", weekly);
    }

    if(!ui->lineEditMonthly->text().isEmpty())
    {
         QString monthly = ui->lineEditMonthly->text();
         settings.setValue("monthly", monthly);
    }

    if(!ui->lineEditYearly->text().isEmpty())
    {
         QString yearly = ui->lineEditYearly->text();
         settings.setValue("yearly", yearly);
    }

    // TODO:
    //if(!ui->lineEditDest->text().isEmpty() && !ui->lineEditSource->text().isEmpty())
    //{
    //    ui->listWidget->setEnabled(false);
    //    ui->treeView->setEnabled(false);
    //    ui->pushButtonStart->setEnabled(false);
    //}
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
    // TODO Remove me
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
    QString pass = ui->linePassword->text();

    QString log = "";
    ui->plainTextEdit->setPlainText(log);

    if(QDir(dest).exists() && QDir(source).exists())
    {
        /*
         * borg create                                                \
         *  --verbose                                                 \
         *  --filter AME                                              \
         *  --list                                                    \
         *  --stats                                                   \
         *  --show-rc                                                 \
         *  --compression lz4                                         \
         *  --exclude-caches                                          \
         *  --exclude '/Users/myzinsky/Desktop/Bilder'                \
         *  --exclude '/Users/myzinsky/EMS/Programming/gem5.traces'   \
         *  --exclude '/Users/myzinsky/Zeugs/steg/vid'                \
         *  ::'{hostname}-{now}'                                      \
         *  /Users/myzinsky                                           \
         */

        time = QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
        QStringList cmdBorg;
        cmdBorg.append("create");
        cmdBorg.append("--filter");
        cmdBorg.append("AME");
        cmdBorg.append("--stats");
        cmdBorg.append("--show-rc");
        cmdBorg.append("--compression");
        cmdBorg.append("lz4");
        cmdBorg.append("--exclude-caches");

        for(int i = 0; i < ui->listWidgetExeptions->count(); i++)
        {
            QString ignore = ui->listWidgetExeptions->item(i)->text();
            cmdBorg.append("--exclude");
            cmdBorg.append(ignore);
        }
        cmdBorg.append(dest + "::{hostname}-{now}");
        cmdBorg.append(source);


        qDebug() << cmdBorg;

        ui->pushButtonStart->setText("Preparing Backup (This may take a while)");
        ui->pushButtonStart->setDisabled(true);
        setTrayIcon(true);

        pBorg = new QProcess(this);

        // Add pass as environment variable:
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("BORG_PASSPHRASE", pass);
        pBorg->setProcessEnvironment(env);

        connect(pBorg,
                SIGNAL(finished(int , QProcess::ExitStatus )),
                this,
                SLOT(on_borgFinished(int , QProcess::ExitStatus )));

        // TODO detect borg:
        pBorg->start(borg, cmdBorg);
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

void derTimeGeraet::locateBorg()
{
    if(QFile::exists("/usr/local/bin/borg"))
    {
        borg = "/usr/local/bin/borg";
    }
    else if(QFile::exists("/usr/bin/borg"))
    {
        borg = "/usr/bin/borg";
    }
    else if(QFile::exists("/bin/borg"))
    {
        borg = "/bin/borg";
    }
    else
    {
        QMessageBox::critical(this,
                              tr("Der Time Gerät"),
                              tr("Cannot find borg"));
        exit(-1);
    }
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

void derTimeGeraet::on_borgFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString log;

    log  =     "Borg Return Code: #####################################\n";
    log += QString::number(exitCode);
    log += "\n\nError String: ##########################################\n";
    log += pBorg->errorString();
    log += "\n\nStandard Error: ########################################\n";
    log += pBorg->readAllStandardError();
    log += "\n\nStandard Output: #######################################\n";
    log += pBorg->readAllStandardOutput();

    ui->plainTextEdit->setPlainText(log);

    if(exitStatus == QProcess::NormalExit && (exitCode == 0 || exitCode == 1))
    {
        prune();
    }
    else
    {
        QMessageBox::critical(this,
                              tr("Der Time Gerät"),
                              tr("Synchronisation Failed"));

        // TODO cleanup again
    }
}

void derTimeGeraet::prune()
{
    QString dest = ui->lineEditDest->text();
    QString source = ui->lineEditSource->text();
    QString pass = ui->linePassword->text();
    QString hourly = ui->lineEditHourly->text();
    QString daily = ui->lineEditDaily->text();
    QString weekly = ui->lineEditWeekly->text();
    QString monthly = ui->lineEditMonthly->text();
    QString yearly = ui->lineEditYearly->text();

    if(QDir(dest).exists() && QDir(source).exists())
    {
        /*
         * borg prune                      \
         * --list                          \
         * --prefix '{hostname}-'          \
         * --show-rc                       \
         * --keep-daily    7               \
         * --keep-weekly   4               \
         * --keep-monthly  6               \
         * --keep-yearly   1               \
         */

        time = QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
        QStringList cmdBorg;
        cmdBorg.append("prune");
        cmdBorg.append("--list");
        cmdBorg.append("--prefix");
        cmdBorg.append("{hostname}-");
        cmdBorg.append("--show-rc");
        cmdBorg.append("--keep-hourly");
        cmdBorg.append(hourly);
        cmdBorg.append("--keep-daily");
        cmdBorg.append(daily);
        cmdBorg.append("--keep-weekly");
        cmdBorg.append(weekly);
        cmdBorg.append("--keep-monthly");
        cmdBorg.append(monthly);
        cmdBorg.append("--keep-yearly");
        cmdBorg.append(yearly);
        cmdBorg.append(dest);

        qDebug() << cmdBorg;

        ui->pushButtonStart->setText("Pruning (This may take a while)");
        ui->pushButtonStart->setDisabled(true);

        pPrune = new QProcess(this);

        // Add pass as environment variable:
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("BORG_PASSPHRASE", pass);
        pPrune->setProcessEnvironment(env);

        connect(pPrune,
                SIGNAL(finished(int , QProcess::ExitStatus )),
                this,
                SLOT(on_pruneFinished(int , QProcess::ExitStatus )));

        // TODO detect borg:
        pPrune->start(borg, cmdBorg);
    }
}


void derTimeGeraet::on_pruneFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString log = ui->plainTextEdit->toPlainText()+"\n\n";

    log +=     "Prune Return Code: #####################################\n";
    log += QString::number(exitCode);
    log += "\n\nError String:###########################################\n";
    log += pBorg->errorString();
    log += "\n\nStandard Error: ########################################\n";
    log += pBorg->readAllStandardError();
    log += "\n\nStandard Output: #######################################\n";
    log += pBorg->readAllStandardOutput();

    ui->plainTextEdit->setPlainText(log);

    loadBackupList();
    ui->pushButtonStart->setEnabled(true);
    ui->pushButtonStart->setText("Start Backup");
    setTrayIcon(false);
}


void derTimeGeraet::on_pushButtonPassword_clicked()
{
    saveSettings();
}

void derTimeGeraet::on_pushButtonPruning_clicked()
{
    saveSettings();
}

void derTimeGeraet::mount(QString key)
{
    QSettings settings(settingsFile, QSettings::NativeFormat);
    QString dest = settings.value("dest", "").toString();
    QString source = ui->lineEditSource->text();
    QString pass = settings.value("pass", "").toString();
    QString dir = "/tmp/derTimeGeraet/";

    // MOUNT:
    if(!QDir(dir).exists())
    {
        QDir().mkdir(dir);
    }

    QStringList cmdMount;
    cmdMount.append("mount");
    cmdMount.append(dest+"::"+key);
    cmdMount.append("/tmp/derTimeGeraet/");
    QProcess pMount;

    qDebug() << cmdMount;

    // Add pass as environment variable:
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("BORG_PASSPHRASE", pass);
    pMount.setProcessEnvironment(env);

    pMount.start(borg, cmdMount);
    pMount.waitForFinished(-1);
    qDebug() << pMount.readAllStandardError();

    dirModel = new QFileSystemModel(this);
    dirModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    dirModel->setRootPath("/");
    ui->treeView->setModel(dirModel);
    ui->treeView->setRootIndex(dirModel->index(dir+source));
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}

void derTimeGeraet::umount()
{
    QSettings settings(settingsFile, QSettings::NativeFormat);
    QString dest = settings.value("dest", "").toString();
    QString pass = settings.value("pass", "").toString();

    // UMOUNT:
    QStringList cmdUmount;
    cmdUmount.append("umount");
    cmdUmount.append("/tmp/derTimeGeraet/");
    QProcess pUmount;

    qDebug() << cmdUmount;

    // Add pass as environment variable:
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("BORG_PASSPHRASE", pass);
    pUmount.setProcessEnvironment(env);

    pUmount.start(borg, cmdUmount);
    pUmount.waitForFinished(-1);
    qDebug() << pUmount.readAllStandardError();
}

void derTimeGeraet::on_listWidget_itemClicked(QListWidgetItem *item)
{
    umount();
    QVariant v = item->data(Qt::UserRole);
    QString key = v.value<QString>();
    mount(key);
}
