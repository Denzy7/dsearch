#include "dbmodel.h"
#include <QMimeDatabase>
#include <QFileInfo>
#include <QDateTime>
#include <QLocale>

namespace DSearch {

DbModel::DbModel()
{
    proxymodel.setSourceModel(this);
    proxymodel.setSortRole(Qt::UserRole);
}

void DbModel::clear()
{
    QStandardItemModel::clear();

    for(size_t i = 0; i < sizeof(colums) / sizeof(colums[0]); i++)
    {
        setHorizontalHeaderItem(i, new QStandardItem(colums[i]));
    }
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
    item->setData(fileInfo.fileName(), Qt::UserRole);
    item->setIcon(icon);
    itemz.append(item);

    item = new QStandardItem(QLocale::system().toString(fileInfo.lastModified(), QLocale::ShortFormat));
    item->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
    item->setData(fileInfo.lastModified(), Qt::UserRole);
    itemz.append(item);

    item = new QStandardItem(QLocale::system().formattedDataSize(fileInfo.size()));
    item->setData(fileInfo.size(), Qt::UserRole);
    item->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
    itemz.append(item);


    item = new QStandardItem(entry.path);
    item->setData(entry.path, Qt::UserRole);
    itemz.append(item);

    appendRow(itemz);

    return 0;
}

}
