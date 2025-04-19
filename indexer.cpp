#include "indexer.h"
#include <QDirIterator>
#include <QStatusBar>
#include <QMessageBox>
#include <QDebug>

#include <unistd.h>

namespace DSearch {

class IndexThreadWorker : public QObject
{
    Q_OBJECT

private:
    Db* m_db;
    Indexer*m_indexer;
    std::vector<QString>* m_paths;
    QDirIterator::IteratorFlags m_iterflags;

public:
    IndexThreadWorker(Db* db, std::vector<QString>* paths, QDirIterator::IteratorFlags iterflags, Indexer* indexer)
    {
        m_indexer = indexer;
        m_db = db;
        m_paths = paths;
        m_iterflags = iterflags;
    }

public slots:
    void doWork()
    {
        std::vector<QString>::iterator it_qstr;
        QThread* me = QThread::currentThread();
        for(it_qstr = m_paths->begin(); it_qstr != m_paths->end(); it_qstr++)
        {
            QDirIterator it_qdir(*it_qstr, m_iterflags);
            DSearch::DbEntry ref;
            while(it_qdir.hasNext())
            {
                if(me->isInterruptionRequested())
                    return;

                if(m_db->Add(it_qdir.next(), &ref))
                {
                    emit OnScan(m_indexer, m_db, &ref);
                    m_indexer->waitinfo.mutex.lock();
                    while(!m_indexer->waitinfo.var)
                        m_indexer->waitinfo.condvar.wait(&m_indexer->waitinfo.mutex);
                    m_indexer->waitinfo.mutex.unlock();
                    m_indexer->waitinfo.var = 0;
                }
            }
        }

        emit IndexFinished(m_indexer, m_db);
    }
signals:
    void IndexFinished(Indexer* indexer, Db* db);
    void OnScan(Indexer* indexer, Db* db, DSearch::DbEntry* entry);

};
int Indexer::Start(Db& db, QDirIterator::IteratorFlags iterflags)
{
    if(m_indexworkerthread.isRunning())
    {
        QMessageBox::critical(nullptr, "Update Database", "Indexer is currently running. Please wait until it's done");
        return 0;
    }

    waitinfo.var = 0;

    IndexThreadWorker* worker = new IndexThreadWorker(&db, &paths, iterflags, this);
    worker->moveToThread(&m_indexworkerthread);

    connect(&m_indexworkerthread, &QThread::finished,
                         worker, &QObject::deleteLater);
    connect(&m_indexworkerthread, &QThread::started,
            worker, &IndexThreadWorker::doWork);

    connect(worker, &IndexThreadWorker::IndexFinished,
                         this, &Indexer::OnIndexFinished);
    connect(worker, &IndexThreadWorker::OnScan,
            this, &Indexer::OnScan);
    m_indexworkerthread.start();
    running = 1;
    return 1;
}


void Indexer::Stop()
{
    if(!m_indexworkerthread.isRunning())
        return;

    waitinfo.var = 0;
    waitinfo.mutex.lock();
    waitinfo.condvar.wakeOne();
    waitinfo.mutex.unlock();

    running = 0;

    m_indexworkerthread.requestInterruption();

    m_indexworkerthread.quit();
    m_indexworkerthread.wait();
}

} // namespace DSearch
#include "indexer.moc"
