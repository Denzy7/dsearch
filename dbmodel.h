#ifndef DBMODEL_H
#define DBMODEL_H

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "db.h"

namespace DSearch {

class DbModel : QStandardItemModel
{
    public:

        QSortFilterProxyModel proxymodel;

        DbModel();
        void clear();
        int AddEntryToModel(DSearch::DbEntry entry);
        
};

}
#endif
