#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <vector>
#include "mainwindow.h"
namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    std::vector<QString>* paths;

    explicit Settings(QWidget *parent = nullptr, MainWindow* mainwindow = nullptr);
    void UpdatePathList();
    ~Settings();

private slots:
    void on_btnAddPath_clicked();
    void on_btnClearSelected_clicked();
    void OnFileWatcherChecked(int state);


private:

    Ui::Settings *ui;
};

#endif // SETTINGS_H
