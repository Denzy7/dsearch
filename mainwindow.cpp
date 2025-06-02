#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "settings.h"
#include "dsearch_config.h"
#include <QDateTime>
#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <QFileSystemWatcher>
#include <QDesktopServices>
#include <QProcess>
#ifdef QT_DBUS_LIB
#include <QtDBus/QDBusReply>
#ifdef Q_OS_UNIX
#include <QDBusUnixFileDescriptor>
#endif
#endif

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <fcntl.h>
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#endif

#include <cstdio>
#include <QIcon>
#include <QLabel>

#include <QThread>
#include <QDebug>
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //showMaximized();

    db = new DSearch::Db("test");

    connect(&fswatcher, &QFileSystemWatcher::directoryChanged,
            this, &MainWindow::onDirectoryChanged);
/*    connect(&fswatcher, &QFileSystemWatcher::fileChanged,*/
            /*this, &MainWindow::onFileChanged);*/

    connect(&m_mainindexer, &DSearch::Indexer::OnScan,
            this, &MainWindow::indexone);
    connect(&m_mainindexer, &DSearch::Indexer::OnIndexFinished,
            this, &MainWindow::indexall);

    connect(ui->tblFiles, &QTableView::customContextMenuRequested,
            this, &MainWindow::onRMBtblFiles);

    ui->tblFiles->horizontalHeader()->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(ui->tblFiles->horizontalHeader(), &QHeaderView::customContextMenuRequested,
            this, &MainWindow::onRMBtblFiles_HorizHdr);
    ui->tblFiles->horizontalHeader()->setSectionsMovable(true);

    //m_indexer.paths.push_back(QDir::homePath());
    m_mainindexer.paths.push_back(QDir::currentPath());

    items_statusbar = new QLabel("Files: 0");
    ui->statusbar->addPermanentWidget(items_statusbar);

    ui->tblFiles->setModel(&dbmodel.proxymodel);
}

MainWindow::~MainWindow()
{
    delete db;
    delete ui;
}


void MainWindow::indexone(DSearch::Indexer* indexer, DSearch::Db* db, DSearch::DbEntry* entry)
{
    if(stopupdaterequested){
        indexer->Stop();
        stopupdaterequested = 0;
        return;
    }
    ui->statusbar->showMessage(entry->path);
    dbmodel.AddEntryToModel(*entry);

    indexer->waitinfo.var = 1;
    indexer->waitinfo.mutex.lock();
    indexer->waitinfo.condvar.wakeOne();
    indexer->waitinfo.mutex.unlock();
}

void MainWindow::indexall(DSearch::Indexer* indexer, DSearch::Db* db)
{
    ui->statusbar->showMessage("Adding paths to watcher...");
    std::vector<QString>::iterator it_qs;
    std::vector<DSearch::DbEntry>::iterator it_entry;
    const std::vector<DSearch::DbEntry>* ents = db->GetEntries();
    for(size_t i = 0; i < ents->size(); i++)
    {
        DSearch::DbEntry* ref;
        db->At(i, &ref);
        fswatcher.addPath(ref->path);
    }
    for(it_qs = m_mainindexer.paths.begin(); it_qs != m_mainindexer.paths.end(); it_qs++)
    {
        fswatcher.addPath(*it_qs);
    }
    ui->statusbar->clearMessage();
    ui->actionStop_Update->setEnabled(false);
    ui->actionUpdate_Database->setEnabled(true);

    items_statusbar->setText(QString("Files: %1").arg(ents->size()));
    m_mainindexer.Stop();
}

void MainWindow::onDirectoryChanged(const QString &path)
{
    QDir dir(path);
    if(!dir.exists()){
        db->Remove(path);
        fswatcher.removePath(path);
        return;
    }

    //TODO: memleak
    DSearch::Indexer* newpathindexer = new DSearch::Indexer;
    newpathindexer->paths.push_back(path);
    newpathindexer->Start(*db, QDirIterator::IteratorFlag::NoIteratorFlags);
    connect(newpathindexer, &DSearch::Indexer::OnScan,
            this, &MainWindow::indexone);
}

void MainWindow::onFileChanged(const QString &path)
{
    //ui->statusbar->showMessage(path);
}

void MainWindow::on_actionSettings_triggered()
{
    Settings settings(this, this);
    settings.paths = &m_mainindexer.paths;
    settings.UpdatePathList();
    settings.exec();
}

void MainWindow::on_actionUpdate_Database_triggered()
{
    db->Clear();
    dbmodel.clear();

    if(fswatcher.files().size())
        fswatcher.removePaths(fswatcher.files());
    if(fswatcher.directories().size())
        fswatcher.removePaths(fswatcher.directories());

    ui->tblFiles->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    m_mainindexer.Start(*db, QDirIterator::IteratorFlag::Subdirectories);

    ui->actionStop_Update->setEnabled(true);
    ui->actionUpdate_Database->setEnabled(false);
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "dsearch v" + QString(DSEARCH_VERSION), "A cross-platform search utility");
}

void MainWindow::on_actionStop_Update_triggered()
{
    stopupdaterequested = 1;
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_ledSearch_textChanged(const QString &arg1)
{
    QSortFilterProxyModel* model = qobject_cast<QSortFilterProxyModel*>(ui->tblFiles->model());
    QString query = ui->ledSearch->text();

    if(model == nullptr)
    {
        ui->statusbar->showMessage("Empty database. Try adding some paths then update it in file menu");
        return;
    }
    model->setFilterCaseSensitivity(searchopts.casesens);
/*    if(!searchopts.fullpath)*/
        /*filepath = fileInfo.fileName();*/
    if(searchopts.regex)
    {
        model->setFilterRegularExpression(query);
    }else {
        model->setFilterFixedString(query);
    }
    //qDebug() << "continue";
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_mainindexer.Stop();
}
void MainWindow::on_actionCase_Sensitive_triggered(bool state)
{
    if(state)
        searchopts.casesens = Qt::CaseSensitive;
    else
        searchopts.casesens = Qt::CaseInsensitive;

    on_ledSearch_textChanged(ui->ledSearch->text());
}

void MainWindow::on_actionRegEx_triggered(bool state)
{
    searchopts.regex = state; 
}

void MainWindow::on_actionSearch_Full_Path_triggered(bool state)
{
    searchopts.fullpath = state;
    on_ledSearch_textChanged(ui->ledSearch->text());
}


void MainWindow::onRMBtblFiles(const QPoint& point)
{
    QModelIndex idx = ui->tblFiles->indexAt(point);

    QMenu contextMenu(tr("rmb_tblFiles"), this);
    QAction* act = contextMenu.addAction("Open");
    act->setIcon(QIcon::fromTheme("document-open"));
    act->setDisabled(idx.row() < 0);
    connect(act, &QAction::triggered,
            this, &MainWindow::onRMBtblFiles_ctx_open);

    act = contextMenu.addAction("Open Directory");
    act->setIcon(QIcon::fromTheme("document-open-folder"));
    act->setDisabled(idx.row() < 0);
    connect(act, &QAction::triggered,
            this, &MainWindow::onRMBtblFiles_ctx_opendir);


    act = contextMenu.addAction("Properties");
    act->setDisabled(idx.row() < 0);
    act->setIcon(QIcon::fromTheme("document-properties"));
    connect(act, &QAction::triggered,
            this, &MainWindow::onRMBtblFiles_ctx_properties);

    QAction *selectedAction = contextMenu.exec(mapToGlobal(point));
}

DSearch::DbEntry MainWindow::getMappedSelectedEntry()
{
    QSortFilterProxyModel* model = qobject_cast<QSortFilterProxyModel*>(ui->tblFiles->model());
    QModelIndex mapped = model->mapToSource(ui->tblFiles->currentIndex());
    DSearch::DbEntry* ref;
    db->At(mapped.row(), &ref);
    return *ref;
}

void MainWindow::onRMBtblFiles_ctx_properties()
{
    DSearch::DbEntry curent = getMappedSelectedEntry();

#ifdef Q_OS_WIN
    SHELLEXECUTEINFOW sei = {0};
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_INVOKEIDLIST;
    sei.hwnd = nullptr;
    sei.lpVerb = L"properties";
    std::wstring wpath = curent.path.toStdWString();
    sei.lpFile = wpath.c_str();
    sei.nShow = SW_SHOW;
    ShellExecuteExW(&sei);
    return;
#endif

#ifdef QT_DBUS_LIB
    if (!FileManager1.isValid()) {
        QMessageBox::critical(this, "DBus Only", "This has only been implemented on dbus systems (eg. Desktop Enviromnents");
        return;
    }

    QDBusReply<void> reply = FileManager1.call("ShowItemProperties", QStringList(curent.path), QString(""));
    if (!reply.isValid())
    {
        QMessageBox::critical(this, "DBus", "Call Failed!\n" + reply.error().message());
    }else {
        return;
    }
#endif

    QMessageBox::critical(this, "Not Impl.", "Unreachabe Code. Not Implemented");
}

void MainWindow::onRMBtblFiles_ctx_open()
{
    DSearch::DbEntry curent = getMappedSelectedEntry();
    if(QDesktopServices::openUrl(QUrl("file:///" + curent.path, QUrl::TolerantMode))){
        return;
    }else {
#ifdef Q_OS_WIN

#endif
    }

    QMessageBox::critical(this, "Not Impl.", "Unreachabe Code. Not Implemented");
}
    
void MainWindow::onRMBtblFiles_ctx_opendir()
{
    DSearch::DbEntry curent = getMappedSelectedEntry();

#ifdef Q_OS_WIN
     QProcess::startDetached("explorer.exe", 
             {QString("/select,\"%1\"").arg(QDir::toNativeSeparators(curent.path))});
     return;
#endif

#ifdef QT_DBUS_LIB
    if (!FileManager1.isValid()) {
        QMessageBox::critical(this, "DBus Only", "This has only been implemented on xdg dbus systems (eg. Desktop Enviromnents");
        return;
    }

    QDBusReply<void> reply = FileManager1.call("ShowItems", QStringList(curent.path), QString(""));
    if (!reply.isValid())
    {
        QMessageBox::critical(this, "DBus", "Call Failed!\n" + reply.error().message());
    }else {
        return;
    }
#endif

    QMessageBox::critical(this, "Not Impl.", "Unreachabe Code. Not Implemented");
}

void MainWindow::onRMBtblFiles_HorizHdr(const QPoint& pont)
{
    QMenu contextMenu("HorizHdr Menu", this);

    QMenu* fit = contextMenu.addMenu("Resize Columns Fit");

    struct ResizeOpts
    {
        const char* name;
        QHeaderView::ResizeMode mode;
    };
    ResizeOpts opts[] = 
    {
        {"Interactive", QHeaderView::Interactive},
        {"Stretch", QHeaderView::Stretch},
        {"Fixed", QHeaderView::Fixed},
        {"Resize to Contents", QHeaderView::ResizeToContents},
    };

    for(ResizeOpts opt : opts)
    {
        QAction* act = fit->addAction(opt.name);
        connect(act, &QAction::triggered,
                [this, opt]()
                {
                this->ui->tblFiles->horizontalHeader()->resizeSections(opt.mode);
                }
               );
    }

    contextMenu.exec(mapToGlobal(pont));

}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange && windowState() & Qt::WindowMaximized){
        ui->tblFiles->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    }

    QMainWindow::changeEvent(event);
}
