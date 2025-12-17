#ifndef INDEXER_H
#define INDEXER_H

#include <QString>
#include <vector>
#include <QDirIterator>
#include <QStatusBar>
#include <QLabel>
#include <QThread>
#include "db.h"
#include "dbmodel.h"

namespace DSearch {

class Indexer : public QObject
{
    Q_OBJECT

private:
    QThread m_indexworkerthread;

public:
    int running;
    QStatusBar* statusbar;
    QLabel* items_statusbar;
    DSearch::DbModel* model;
    std::vector<QString> paths;
    int Start(Db& db, QDirIterator::IteratorFlags iterflags);
    void Stop();

signals:
    void OnIndexFinished(Indexer* indexer, Db* db);
    void OnScan(Indexer* indexer,Db* db, DSearch::DbEntry* entry);
};

} // namespace DSearch

#endif // INDEXER_H
