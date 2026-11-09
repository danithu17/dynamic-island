#include "ShelfManager.h"
#include "../../core/DatabaseManager.h"
#include "../../core/AppState.h"
#include <QFileInfo>
#include <QDesktopServices>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QClipboard>
#include <QGuiApplication>
#include <QUrl>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace Aura {

ShelfManager &ShelfManager::instance() {
    static ShelfManager s_instance;
    return s_instance;
}

ShelfManager::ShelfManager(QObject *parent)
    : QObject(parent) {
    loadFromDatabase();
}

void ShelfManager::loadFromDatabase() {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery q("SELECT id, type, title, file_path, data, is_pinned, created_at FROM shelf_items ORDER BY is_pinned DESC, id DESC", db);
    std::vector<ShelfItem> items;
    while (q.next()) {
        ShelfItem item;
        item.id = q.value(0).toInt();
        item.type = q.value(1).toString();
        item.title = q.value(2).toString();
        item.filePath = q.value(3).toString();
        item.data = q.value(4).toString();
        item.isPinned = (q.value(5).toInt() == 1);
        item.createdAt = q.value(6).toDateTime();
        items.push_back(std::move(item));
    }
    m_model.setItems(std::move(items));
}

void ShelfManager::addFile(const QString &localFilePath) {
    QFileInfo fi(localFilePath);
    if (!fi.exists()) return;

    ShelfItem item;
    QString ext = fi.suffix().toLower();
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "gif" || ext == "webp" || ext == "svg" || ext == "ico") {
        item.type = "image";
    } else {
        item.type = "file";
    }
    item.title = fi.fileName();
    item.filePath = fi.absoluteFilePath();
    item.data = "";
    item.isPinned = false;
    item.createdAt = QDateTime::currentDateTime();

    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("INSERT INTO shelf_items (type, title, file_path, data, is_pinned, created_at) "
                  "VALUES (:type, :title, :path, :data, :pinned, :created)");
        q.bindValue(":type", item.type);
        q.bindValue(":title", item.title);
        q.bindValue(":path", item.filePath);
        q.bindValue(":data", item.data);
        q.bindValue(":pinned", 0);
        q.bindValue(":created", item.createdAt.toString(Qt::ISODate));
        if (q.exec()) {
            item.id = q.lastInsertId().toInt();
            m_model.addItem(item);
            AppState::instance().showToast("Added to Shelf: " + item.title, "success");
        }
    }
}

void ShelfManager::addDropUrls(const QList<QUrl> &urls) {
    for (const QUrl &url : urls) {
        if (url.isLocalFile()) {
            addFile(url.toLocalFile());
        } else {
            ShelfItem item;
            item.type = "link";
            item.title = url.toString();
            item.filePath = "";
            item.data = url.toString();
            item.isPinned = false;
            item.createdAt = QDateTime::currentDateTime();

            auto db = DatabaseManager::instance().database();
            if (db.isOpen()) {
                QSqlQuery q(db);
                q.prepare("INSERT INTO shelf_items (type, title, file_path, data, is_pinned, created_at) "
                          "VALUES (:type, :title, :path, :data, :pinned, :created)");
                q.bindValue(":type", item.type);
                q.bindValue(":title", item.title);
                q.bindValue(":path", item.filePath);
                q.bindValue(":data", item.data);
                q.bindValue(":pinned", 0);
                q.bindValue(":created", item.createdAt.toString(Qt::ISODate));
                if (q.exec()) {
                    item.id = q.lastInsertId().toInt();
                    m_model.addItem(item);
                }
            }
        }
    }
}

void ShelfManager::addText(const QString &text) {
    if (text.trimmed().isEmpty()) return;

    ShelfItem item;
    item.type = "text";
    item.title = text.left(32).trimmed();
    item.filePath = "";
    item.data = text;
    item.isPinned = false;
    item.createdAt = QDateTime::currentDateTime();

    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("INSERT INTO shelf_items (type, title, file_path, data, is_pinned, created_at) "
                  "VALUES (:type, :title, :path, :data, :pinned, :created)");
        q.bindValue(":type", item.type);
        q.bindValue(":title", item.title);
        q.bindValue(":path", item.filePath);
        q.bindValue(":data", item.data);
        q.bindValue(":pinned", 0);
        q.bindValue(":created", item.createdAt.toString(Qt::ISODate));
        if (q.exec()) {
            item.id = q.lastInsertId().toInt();
            m_model.addItem(item);
            AppState::instance().showToast("Snippet added to Shelf", "success");
        }
    }
}

void ShelfManager::openItem(int id) {
    for (int i = 0; i < m_model.rowCount(); ++i) {
        const ShelfItem *item = m_model.getItemAt(i);
        if (item && item->id == id) {
            if (!item->filePath.isEmpty()) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(item->filePath));
            } else if (item->type == "link") {
                QDesktopServices::openUrl(QUrl(item->data));
            } else {
                copyPath(id);
            }
            break;
        }
    }
}

void ShelfManager::copyPath(int id) {
    for (int i = 0; i < m_model.rowCount(); ++i) {
        const ShelfItem *item = m_model.getItemAt(i);
        if (item && item->id == id) {
            QString toCopy = !item->filePath.isEmpty() ? item->filePath : item->data;
            QGuiApplication::clipboard()->setText(toCopy);
            AppState::instance().showToast("Copied to clipboard", "info");
            break;
        }
    }
}

void ShelfManager::togglePin(int id) {
    for (int i = 0; i < m_model.rowCount(); ++i) {
        const ShelfItem *item = m_model.getItemAt(i);
        if (item && item->id == id) {
            bool newPinned = !item->isPinned;
            auto db = DatabaseManager::instance().database();
            if (db.isOpen()) {
                QSqlQuery q(db);
                q.prepare("UPDATE shelf_items SET is_pinned = :pinned WHERE id = :id");
                q.bindValue(":pinned", newPinned ? 1 : 0);
                q.bindValue(":id", id);
                if (q.exec()) {
                    m_model.updateItemPinned(id, newPinned);
                }
            }
            break;
        }
    }
}

void ShelfManager::removeItem(int id) {
    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("DELETE FROM shelf_items WHERE id = :id");
        q.bindValue(":id", id);
        if (q.exec()) {
            m_model.removeItemById(id);
        }
    }
}

void ShelfManager::clearUnpinned() {
    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.exec("DELETE FROM shelf_items WHERE is_pinned = 0");
        loadFromDatabase();
    }
}

void ShelfManager::revealInExplorer(int id) {
    for (int i = 0; i < m_model.rowCount(); ++i) {
        const ShelfItem *item = m_model.getItemAt(i);
        if (item && item->id == id && !item->filePath.isEmpty()) {
            QString nativePath = QDir::toNativeSeparators(item->filePath);
            QProcess::startDetached("explorer.exe", QStringList() << "/select," << nativePath);
            break;
        }
    }
}

void ShelfManager::openTrayFolder() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString trayDir = appData + "/tray";
    QDir().mkpath(trayDir);
    QDesktopServices::openUrl(QUrl::fromLocalFile(trayDir));
}

void ShelfManager::setSearchQuery(const QString &query) {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QString sql = "SELECT id, type, title, file_path, data, is_pinned, created_at FROM shelf_items";
    if (!query.trimmed().isEmpty()) {
        sql += " WHERE title LIKE :q OR file_path LIKE :q OR data LIKE :q";
    }
    sql += " ORDER BY is_pinned DESC, id DESC";

    QSqlQuery q(db);
    q.prepare(sql);
    if (!query.trimmed().isEmpty()) {
        q.bindValue(":q", "%" + query.trimmed() + "%");
    }
    if (q.exec()) {
        std::vector<ShelfItem> items;
        while (q.next()) {
            ShelfItem item;
            item.id = q.value(0).toInt();
            item.type = q.value(1).toString();
            item.title = q.value(2).toString();
            item.filePath = q.value(3).toString();
            item.data = q.value(4).toString();
            item.isPinned = (q.value(5).toInt() == 1);
            item.createdAt = q.value(6).toDateTime();
            items.push_back(std::move(item));
        }
        m_model.setItems(std::move(items));
    }
}

} // namespace Aura
