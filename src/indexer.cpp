#include "indexer.h"
#include <QDirIterator>
#include <QStatusBar>
#include <QMessageBox>
#include <QDebug>

#include <QElapsedTimer>

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
        QElapsedTimer tt;
        tt.start();
        for(it_qstr = m_paths->begin(); it_qstr != m_paths->end(); it_qstr++)
        {
            QDirIterator it_qdir(*it_qstr, m_iterflags);
            DSearch::DbEntry ref;
            while(it_qdir.hasNext())
            {
                if(me->isInterruptionRequested()){
                    emit IndexFinished(m_indexer, m_db);
                    return;
                }

                if(m_db->Add(it_qdir.next(), &ref))
                {
                    if(tt.elapsed() > 10)
                    {
                        m_indexer->statusbar->showMessage(ref.path);
                        m_indexer->items_statusbar->setText(QString("Files: %1").arg(m_db->GetEntries()->size()));
                        tt.restart();
                    }
                    m_indexer->model->AddEntryToModel(ref);
                    emit OnScan(m_indexer, m_db, &ref);
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

    m_indexworkerthread.requestInterruption();

    running = 0;

    /* you really can't "quit" a
     * thread but qt needs this so thread
     * data is freed
     */
    m_indexworkerthread.quit();
    m_indexworkerthread.wait();
}

} // namespace DSearch
#include "indexer.moc"
