#include "settings.h"
#include "ui_settings.h"

#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QCheckBox>
Settings::Settings(QWidget *parent, MainWindow* mainwindow)
    : QDialog(parent)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);
    ui->lstPaths->setModel(new QStandardItemModel(this));
    ui->chkFileWatcher->setCheckState(
            mainwindow->fswatcher.directories().size() > 0 || mainwindow->fswatcher.directories().size() > 0 ?
            Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    connect(ui->chkFileWatcher, &QCheckBox::stateChanged,
            this, &Settings::OnFileWatcherChecked);
    ui->lblWatcherDirs->setText(QString("Directories: %1").arg(mainwindow->fswatcher.directories().size()));
    ui->lblWatcherFiles->setText(QString("Files: %1").arg(mainwindow->fswatcher.files().size()));
}

void Settings::UpdatePathList()
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->lstPaths->model());
    model->clear();

    std::vector<QString>::iterator it;
    for(it = paths->begin(); it != paths->end(); it++)
    {
        model->appendRow(new QStandardItem(*it));
    }
}

Settings::~Settings()
{
    delete ui->lstPaths->model();
    delete ui;
}

void Settings::on_btnAddPath_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setWindowTitle("Select Directory Path");

    if(dialog.exec() != QDialog::Accepted)
        return;

    // seems we can't select multiple directories
    QString path = dialog.selectedFiles()[0];
    std::vector<QString>::iterator it_qstr;

    for(it_qstr = paths->begin(); it_qstr != paths->end(); it_qstr++)
    {
        // dont need if: exists, recursive
        if(path == *it_qstr || path.contains(*it_qstr))
            return;
    }
    paths->push_back(path);

    UpdatePathList();
}


void Settings::on_btnClearSelected_clicked()
{
    QModelIndexList indices = ui->lstPaths->selectionModel()->selectedIndexes();
    if(indices.isEmpty())
    {
        QMessageBox::critical(this, "No Entry Selected", "You must select a path to remove");
        return;
    }
    QModelIndexList::iterator it;
    for(it = indices.begin(); it != indices.end(); it++)
    {
        paths->erase(paths->begin() + it->row());
    }
    UpdatePathList();
}


void Settings::OnFileWatcherChecked(int state)
{
    qDebug() << "not impl.";
}

