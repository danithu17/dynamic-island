#include "DatabaseManager.h"
#include "Paths.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace Aura {

DatabaseManager &DatabaseManager::instance() {
    static DatabaseManager s_instance;
    return s_instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent) {
}

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::init() {
    QMutexLocker locker(&m_mutex);
    if (m_initialized) return true;

    QString dbPath = Paths::databasePath();
    m_db = QSqlDatabase::addDatabase("QSQLITE", "aura_island_connection");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "Failed to open SQLite database at" << dbPath << ":" << m_db.lastError().text();
        return false;
    }

    m_initialized = createTables();
    return m_initialized;
}

void DatabaseManager::close() {
    QMutexLocker locker(&m_mutex);
    if (m_db.isOpen()) {
        m_db.close();
    }
    m_initialized = false;
}

QSqlDatabase DatabaseManager::database() {
    return m_db;
}

bool DatabaseManager::execute(const QString &queryStr) {
    QMutexLocker locker(&m_mutex);
    if (!m_db.isOpen()) return false;
    QSqlQuery q(m_db);
    if (!q.exec(queryStr)) {
        qWarning() << "SQL execute failed:" << q.lastError().text() << "Query:" << queryStr;
        return false;
    }
    return true;
}

bool DatabaseManager::createTables() {
    QSqlQuery q(m_db);

    // Shelf items table
    const QString shelfTable = R"(
        CREATE TABLE IF NOT EXISTS shelf_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            type TEXT NOT NULL,
            title TEXT NOT NULL,
            file_path TEXT,
            data TEXT,
            is_pinned INTEGER DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    // Clipboard history table
    const QString clipboardTable = R"(
        CREATE TABLE IF NOT EXISTS clipboard_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            type TEXT NOT NULL,
            text_data TEXT NOT NULL,
            is_pinned INTEGER DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    // Notes and snippets table
    const QString notesTable = R"(
        CREATE TABLE IF NOT EXISTS notes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            language TEXT DEFAULT 'markdown',
            code TEXT NOT NULL,
            tags TEXT,
            is_pinned INTEGER DEFAULT 0,
            project_path TEXT DEFAULT '',
            is_snippet INTEGER DEFAULT 0,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    // Scratchpad table
    const QString scratchpadTable = R"(
        CREATE TABLE IF NOT EXISTS scratchpad (
            id INTEGER PRIMARY KEY CHECK (id = 1),
            content TEXT DEFAULT '',
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    // Settings key-value table
    const QString settingsTable = R"(
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        );
    )";

    if (!q.exec(shelfTable) || !q.exec(clipboardTable) || !q.exec(notesTable) || !q.exec(scratchpadTable) || !q.exec(settingsTable)) {
        qCritical() << "Failed to create database tables:" << q.lastError().text();
        return false;
    }

    // Run schema migrations for existing databases
    q.exec("ALTER TABLE notes ADD COLUMN project_path TEXT DEFAULT ''");
    q.exec("ALTER TABLE notes ADD COLUMN is_snippet INTEGER DEFAULT 0");

    return true;
}

} // namespace Aura
