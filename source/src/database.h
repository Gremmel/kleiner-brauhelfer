#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QString>
#include <QSqlQuery>

class Database : public QObject
{
    Q_OBJECT

public:
    static bool transaction();
    static bool commit();
    static bool rollback();
    static int SudKopieren(const QString &id, const QString& name, bool full);

private:
    static void error(const QSqlQuery& query);
};

#endif // DATABASE_H
