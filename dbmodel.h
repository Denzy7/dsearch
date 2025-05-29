#ifndef DBMODEL_H
#define DBMODEL_H

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "db.h"

namespace DSearch {

class DbModel : QStandardItemModel
{
    public:
        QString colums[4] = 
        {
            "File",
            "Last Modified",
            "Size",
            "Path"
        };

        QSortFilterProxyModel proxymodel;

        DbModel();
        void clear();
        int AddEntryToModel(DSearch::DbEntry entry);
        
};

}
#endif
