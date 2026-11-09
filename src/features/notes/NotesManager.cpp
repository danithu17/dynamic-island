#include "NotesManager.h"
#include "../../core/DatabaseManager.h"
#include "../../core/AppState.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace Aura {

NotesManager &NotesManager::instance() {
    static NotesManager s_instance;
    return s_instance;
}

NotesManager::NotesManager(QObject *parent)
    : QObject(parent) {
    loadFromDatabase();
}

void NotesManager::loadFromDatabase() {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery q("SELECT id, title, language, code, tags, is_pinned, updated_at FROM notes ORDER BY is_pinned DESC, updated_at DESC", db);
    std::vector<NoteItem> items;
    while (q.next()) {
        NoteItem item;
        item.id = q.value(0).toInt();
        item.title = q.value(1).toString();
        item.language = q.value(2).toString();
        item.code = q.value(3).toString();
        item.tags = q.value(4).toString();
        item.isPinned = (q.value(5).toInt() == 1);
        item.updatedAt = q.value(6).toDateTime();
        items.push_back(std::move(item));
    }
    m_model.setItems(std::move(items));
}

void NotesManager::setSearchQuery(const QString &query) {
    if (m_searchQuery != query) {
        m_searchQuery = query;
        m_model.setFilter(query);
        emit searchQueryChanged();
    }
}

void NotesManager::saveNote(int id, const QString &title, const QString &language, const QString &code, const QString &tags) {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QDateTime now = QDateTime::currentDateTime();

    if (id == 0) {
        // Create new
        NoteItem item;
        item.title = title.isEmpty() ? "Untitled Note" : title;
        item.language = language.isEmpty() ? "plaintext" : language;
        item.code = code;
        item.tags = tags;
        item.isPinned = false;
        item.updatedAt = now;

        QSqlQuery q(db);
        q.prepare("INSERT INTO notes (title, language, code, tags, is_pinned, updated_at) "
                  "VALUES (:title, :lang, :code, :tags, 0, :updated)");
        q.bindValue(":title", item.title);
        q.bindValue(":lang", item.language);
        q.bindValue(":code", item.code);
        q.bindValue(":tags", item.tags);
        q.bindValue(":updated", item.updatedAt.toString(Qt::ISODate));
        if (q.exec()) {
            item.id = q.lastInsertId().toInt();
            m_model.addItem(item);
            AppState::instance().showToast("Note created", "success");
        }
    } else {
        // Update existing
        QSqlQuery q(db);
        q.prepare("UPDATE notes SET title = :title, language = :lang, code = :code, tags = :tags, updated_at = :updated WHERE id = :id");
        q.bindValue(":title", title);
        q.bindValue(":lang", language);
        q.bindValue(":code", code);
        q.bindValue(":tags", tags);
        q.bindValue(":updated", now.toString(Qt::ISODate));
        q.bindValue(":id", id);
        if (q.exec()) {
            NoteItem updated;
            updated.id = id;
            updated.title = title;
            updated.language = language;
            updated.code = code;
            updated.tags = tags;
            updated.updatedAt = now;
            m_model.updateItem(updated);
            AppState::instance().showToast("Note saved", "info");
        }
    }
}

void NotesManager::copyCode(int id) {
    for (int i = 0; i < m_model.rowCount(); ++i) {
        const NoteItem *item = m_model.getItemAt(i);
        if (item && item->id == id) {
            QGuiApplication::clipboard()->setText(item->code);
            AppState::instance().showToast("Code copied to clipboard", "success");
            break;
        }
    }
}

void NotesManager::togglePin(int id) {
    for (int i = 0; i < m_model.rowCount(); ++i) {
        const NoteItem *item = m_model.getItemAt(i);
        if (item && item->id == id) {
            bool newPinned = !item->isPinned;
            auto db = DatabaseManager::instance().database();
            if (db.isOpen()) {
                QSqlQuery q(db);
                q.prepare("UPDATE notes SET is_pinned = :pinned WHERE id = :id");
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

void NotesManager::deleteNote(int id) {
    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("DELETE FROM notes WHERE id = :id");
        q.bindValue(":id", id);
        if (q.exec()) {
            m_model.removeItemById(id);
            AppState::instance().showToast("Note deleted", "info");
        }
    }
}

} // namespace Aura
