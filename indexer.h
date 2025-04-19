#ifndef INDEXER_H
#define INDEXER_H

#include <QString>
#include <vector>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QDirIterator>
#include "db.h"
#include "dbmodel.h"

namespace DSearch {

struct WaitInfo
{
    QMutex mutex;
    QWaitCondition condvar;
    int var;
};

class Indexer : public QObject
{
    Q_OBJECT

private:
    QThread m_indexworkerthread;

public:
    int running;
    WaitInfo waitinfo;
    std::vector<QString> paths;
    int Start(Db& db, QDirIterator::IteratorFlags iterflags);
    void Stop();

signals:
    void OnIndexFinished(Indexer* indexer, Db* db);
    void OnScan(Indexer* indexer,Db* db, DSearch::DbEntry* entry);
};

} // namespace DSearch

#endif // INDEXER_H
