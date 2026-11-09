#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>

namespace Aura {

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager &instance();

    bool init();
    void close();
    QSqlDatabase database();

    // Helper queries
    bool execute(const QString &queryStr);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;

    bool createTables();

    QSqlDatabase m_db;
    QMutex m_mutex;
    bool m_initialized = false;
};

} // namespace Aura
