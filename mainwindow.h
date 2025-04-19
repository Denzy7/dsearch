#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QFileSystemWatcher>
#ifdef QT_DBUS_LIB
#include <QtDBus/QDBusInterface>
#endif

#include <QMutex>

#include "indexer.h"
#include "db.h"
#include "dbmodel.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct SearchOptions
{
    Qt::CaseSensitivity casesens = Qt::CaseInsensitive;
    int fullpath = 0;
    bool regex = 0;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();
    void on_ledSearch_textChanged(const QString &arg1);

    void on_actionUpdate_Database_triggered();
    void on_actionStop_Update_triggered();

    void on_actionSettings_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionRegEx_triggered(bool state);
    void on_actionCase_Sensitive_triggered(bool state);
    void on_actionSearch_Full_Path_triggered(bool state);

    void onRMBtblFiles_ctx_opendir();
    void onRMBtblFiles_ctx_open();
    void onRMBtblFiles_ctx_properties();

private:
#ifdef QT_DBUS_LIB
    //TODO: this is probably illegal 
    QDBusInterface FileManager1 = QDBusInterface(
            "org.freedesktop.FileManager1",
            "/org/freedesktop/FileManager1",
            "org.freedesktop.FileManager1",
            QDBusConnection::sessionBus());
#endif
    Ui::MainWindow *ui;
    QLabel* items_statusbar;

    DSearch::Indexer m_mainindexer;
    DSearch::Db* db;
    DSearch::DbModel dbmodel;

    SearchOptions searchopts;
    DSearch::DbEntry getMappedSelectedEntry();

public slots:
    void indexone(DSearch::Indexer* indexer, DSearch::Db* db, DSearch::DbEntry* entry);
    void indexall(DSearch::Indexer* indexer, DSearch::Db* db);

    void onDirectoryChanged(const QString &path);
    void onFileChanged(const QString &path);

    void onRMBtblFiles(const QPoint& pont);
    void onRMBtblFiles_HorizHdr(const QPoint& pont);
public:
    QFileSystemWatcher fswatcher;

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

};
#endif // MAINWINDOW_H
