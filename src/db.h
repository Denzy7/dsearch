#ifndef DB_H
#define DB_H

#include <QString>
#include <QObject>
#include <QMutex>
#include <vector>

namespace DSearch {
enum DbEntryType
{
    FILE,
    DIRECTORY
};

struct DbEntry
{
    DbEntryType type;
    QString path;
};

class Db
{
private:
    QString m_dbpath;
    QMutex m_entmutex;
    std::vector<DbEntry> m_entires;

public:

    Db(const QString& dbpath);
    int Load();
    const std::vector<DbEntry>* GetEntries();
    void Clear();
    int Add(const QString& path, DbEntry* ref);
    int At(const size_t i, DbEntry** ref);
    int Remove(const QString& path);
    int Write(const QString& writepath = nullptr);
    int Search(const QString& keyword, DbEntry* entry);
};

} // namespace DSearch
#endif // DB_H
