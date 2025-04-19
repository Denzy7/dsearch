#include "dbmodel.h"
#include <QMimeDatabase>
#include <QFileInfo>
#include <QDateTime>
#include <QLocale>

namespace DSearch {

DbModel::DbModel()
{
    proxymodel.setSourceModel(this);
}

void DbModel::clear()
{
    QStandardItemModel::clear();

    setHorizontalHeaderLabels({"File" , "Last Modified", "Size", "Path", });
    proxymodel.setFilterKeyColumn(0);
}
int DbModel::AddEntryToModel(DSearch::DbEntry entry)
{
    QMimeDatabase mimedb;
    QMimeType mime = mimedb.mimeTypeForFile(entry.path);

    QIcon icon = QIcon::fromTheme(mime.iconName());

    if(icon.isNull())
        icon = QIcon::fromTheme("unknown");

    QFileInfo fileInfo(entry.path);

    QList<QStandardItem*> itemz;

    QStandardItem* item = new QStandardItem(fileInfo.fileName());
    item->setIcon(icon);
    itemz.append(item);

    item = new QStandardItem(QLocale::system().toString(fileInfo.lastModified(), QLocale::ShortFormat));
    item->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
    itemz.append(item);

    item = new QStandardItem(
            QString("%1").arg(fileInfo.size())
            );
    item->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
    itemz.append(item);


    item = new QStandardItem(entry.path);
    itemz.append(item);

    appendRow(itemz);

    return 0;
}

}
