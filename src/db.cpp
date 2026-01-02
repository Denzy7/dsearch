#include "db.h"
#include <fstream>
#include <iostream>

#include <QMessageBox>
#include <QDir>
#include <QDebug>

namespace DSearch {

Db::Db(const QString& dbpath) {
    m_dbpath = dbpath;
}

int Db::Write(const QString& writepath)
{
    QString actpath = writepath;
    if(writepath == nullptr)
        actpath = m_dbpath;

    std::string a = actpath.toStdString();

    return 1;
}
int Db::Load()
{
    m_entires.clear();
    std::ifstream dbfile(m_dbpath.toStdString());
    if(!dbfile.is_open())
    {
        QMessageBox::critical(nullptr, "Cannot load databse file", "File not found");
        return 0;
    }

    std::string line;
    while(!std::getline(dbfile, line))
    {
        //Add(QString(line.c_str()));
    }

    return 1;
}

int Db::Remove(const QString& path)
{
    std::vector<DbEntry>::iterator it;
    for(it = m_entires.begin(); it != m_entires.end(); it++)
    {
        if(it->path == path)
        {
            m_entires.erase(it);
            return 1;
        }
    }
    return 0;
}

const std::vector<DbEntry>* Db::GetEntries()
{
    return &m_entires;
}

void Db::Clear()
{
    m_entmutex.lock();
    m_entires.clear();
    m_entmutex.unlock();
}

int Db::At(const size_t i, DbEntry** ref)
{
    *ref = &m_entires.at(i);
    return 1;
}

int Db::Add(const QString& path, DbEntry* ref)
{
    //m_entmutex.lock();

    std::vector<DbEntry>::iterator it_dbent;

    if(path.isEmpty())
        return 0;

    if(path.back() == '.' || path.back() == '/')
        return 0;

    for(it_dbent = m_entires.begin(); it_dbent != m_entires.end(); it_dbent++)
    {
        if(it_dbent->path == path)
            return 0;
    }


    ref->path = path;
    ref->type = QDir(path).exists() ? DIRECTORY : FILE;
    //std::cout << path.toStdString() << " " << entry.type << std::endl;
    m_entires.push_back(*ref);

    //m_entmutex.unlock();
    return 1;
}

} // namespace DSearch
