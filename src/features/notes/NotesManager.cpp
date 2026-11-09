#include "NotesManager.h"
#include "../../core/DatabaseManager.h"
#include "../../core/AppState.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QTime>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

namespace Aura {

NotesManager &NotesManager::instance() {
    static NotesManager s_instance;
    return s_instance;
}

NotesManager::NotesManager(QObject *parent)
    : QObject(parent) {
    loadFromDatabase();
    loadScratchpad();
}

void NotesManager::loadFromDatabase() {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery q("SELECT id, title, language, code, tags, is_pinned, project_path, is_snippet, updated_at "
                "FROM notes ORDER BY is_pinned DESC, updated_at DESC", db);
    std::vector<NoteItem> items;
    while (q.next()) {
        NoteItem item;
        item.id = q.value(0).toInt();
        item.title = q.value(1).toString();
        item.language = q.value(2).toString();
        item.code = q.value(3).toString();
        item.tags = q.value(4).toString();
        item.isPinned = (q.value(5).toInt() == 1);
        item.projectPath = q.value(6).toString();
        item.isSnippet = (q.value(7).toInt() == 1);
        item.updatedAt = q.value(8).toDateTime();
        items.push_back(std::move(item));
    }
    m_model.setItems(std::move(items));
    emit noteCountChanged();
}

void NotesManager::loadScratchpad() {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery q("SELECT content FROM scratchpad WHERE id = 1", db);
    if (q.next()) {
        m_scratchpadContent = q.value(0).toString();
        emit scratchpadContentChanged();
    }
}

void NotesManager::saveScratchpadToDb() {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery q(db);
    q.prepare("INSERT OR REPLACE INTO scratchpad (id, content, updated_at) VALUES (1, :c, CURRENT_TIMESTAMP)");
    q.bindValue(":c", m_scratchpadContent);
    q.exec();
}

void NotesManager::setSearchQuery(const QString &query) {
    if (m_searchQuery != query) {
        m_searchQuery = query;
        m_model.setFilter(query);
        emit searchQueryChanged();
    }
}

void NotesManager::setCategoryFilter(const QString &category) {
    if (m_categoryFilter != category) {
        m_categoryFilter = category;
        m_model.setCategoryFilter(category);
        emit categoryFilterChanged();
    }
}

void NotesManager::setScratchpadContent(const QString &text) {
    if (m_scratchpadContent != text) {
        m_scratchpadContent = text;
        emit scratchpadContentChanged();
        saveScratchpadToDb();
    }
}

void NotesManager::clearScratchpad() {
    m_scratchpadContent.clear();
    emit scratchpadContentChanged();
    saveScratchpadToDb();
    AppState::instance().showToast("Scratchpad cleared", "info");
}

void NotesManager::setActiveProject(const QString &proj) {
    if (m_activeProject != proj) {
        m_activeProject = proj;
        m_model.setProjectFilter(proj);
        emit activeProjectChanged();
    }
}

void NotesManager::saveNote(int id, const QString &title, const QString &language,
                            const QString &code, const QString &tags,
                            const QString &projectPath, bool isSnippet) {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QDateTime now = QDateTime::currentDateTime();

    if (id == 0) {
        // Create new
        NoteItem item;
        item.title = title.isEmpty() ? (isSnippet ? "Untitled Snippet" : "Untitled Note") : title;
        item.language = language.isEmpty() ? (isSnippet ? "javascript" : "markdown") : language;
        item.code = code;
        item.tags = tags;
        item.isPinned = false;
        item.projectPath = projectPath;
        item.isSnippet = isSnippet;
        item.updatedAt = now;

        QSqlQuery q(db);
        q.prepare("INSERT INTO notes (title, language, code, tags, is_pinned, project_path, is_snippet, updated_at) "
                  "VALUES (:title, :lang, :code, :tags, 0, :proj, :snip, :updated)");
        q.bindValue(":title", item.title);
        q.bindValue(":lang", item.language);
        q.bindValue(":code", item.code);
        q.bindValue(":tags", item.tags);
        q.bindValue(":proj", item.projectPath);
        q.bindValue(":snip", item.isSnippet ? 1 : 0);
        q.bindValue(":updated", item.updatedAt.toString(Qt::ISODate));
        if (q.exec()) {
            item.id = q.lastInsertId().toInt();
            m_model.addItem(item);
            AppState::instance().showToast(isSnippet ? "Snippet created" : "Note created", "success");
            emit noteCountChanged();
        }
    } else {
        // Update existing
        QSqlQuery q(db);
        q.prepare("UPDATE notes SET title = :title, language = :lang, code = :code, tags = :tags, "
                  "project_path = :proj, is_snippet = :snip, updated_at = :updated WHERE id = :id");
        q.bindValue(":title", title);
        q.bindValue(":lang", language);
        q.bindValue(":code", code);
        q.bindValue(":tags", tags);
        q.bindValue(":proj", projectPath);
        q.bindValue(":snip", isSnippet ? 1 : 0);
        q.bindValue(":updated", now.toString(Qt::ISODate));
        q.bindValue(":id", id);
        if (q.exec()) {
            NoteItem updated;
            updated.id = id;
            updated.title = title;
            updated.language = language;
            updated.code = code;
            updated.tags = tags;
            updated.projectPath = projectPath;
            updated.isSnippet = isSnippet;
            updated.updatedAt = now;
            m_model.updateItem(updated);
            AppState::instance().showToast("Saved", "info");
        }
    }
}

void NotesManager::quickCapture(const QString &text) {
    if (text.trimmed().isEmpty()) return;

    QString trimmed = text.trimmed();
    QString firstLine = trimmed.section('\n', 0, 0);
    if (firstLine.startsWith('#')) {
        firstLine = firstLine.remove(0, 1).trimmed();
    }
    if (firstLine.length() > 36) firstLine = firstLine.left(33) + "...";

    saveNote(0, firstLine, "markdown", trimmed, "quick", m_activeProject, false);
    AppState::instance().showToast("Quick Note Captured!", "success");
}

QString NotesManager::expandVariables(const QString &templateCode) const {
    QString expanded = templateCode;
    expanded.replace("{date}", QDate::currentDate().toString("yyyy-MM-dd"));
    expanded.replace("{time}", QTime::currentTime().toString("HH:mm:ss"));
    expanded.replace("{uuid}", QUuid::createUuid().toString(QUuid::WithoutBraces));
    expanded.replace("{clipboard}", QGuiApplication::clipboard()->text());
    expanded.replace("{cursor}", ""); // Remove cursor indicator
    return expanded;
}

void NotesManager::copyCode(int id) {
    const NoteItem *item = m_model.getItemById(id);
    if (!item) return;

    QGuiApplication::clipboard()->setText(item->code);
    AppState::instance().showToast("Copied to clipboard", "success");
}

void NotesManager::copySnippetWithVariables(int id) {
    const NoteItem *item = m_model.getItemById(id);
    if (!item) return;

    QString expanded = expandVariables(item->code);
    QGuiApplication::clipboard()->setText(expanded);
    AppState::instance().showToast("Snippet expanded & copied", "success");
}

void NotesManager::toggleTodoCheckbox(int id, int todoIndex) {
    const NoteItem *item = m_model.getItemById(id);
    if (!item) return;

    QString code = item->code;
    int pos = 0;
    int currentIdx = 0;

    while ((pos = code.indexOf("- [", pos)) != -1) {
        if (pos + 4 <= code.length()) {
            QChar state = code[pos + 3];
            if (state == ' ' || state == 'x' || state == 'X') {
                if (currentIdx == todoIndex) {
                    // Toggle
                    code[pos + 3] = (state == ' ') ? 'x' : ' ';
                    saveNote(item->id, item->title, item->language, code, item->tags, item->projectPath, item->isSnippet);
                    return;
                }
                currentIdx++;
            }
        }
        pos += 4;
    }
}

void NotesManager::togglePin(int id) {
    const NoteItem *item = m_model.getItemById(id);
    if (!item) return;

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
}

void NotesManager::deleteNote(int id) {
    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("DELETE FROM notes WHERE id = :id");
        q.bindValue(":id", id);
        if (q.exec()) {
            m_model.removeItemById(id);
            AppState::instance().showToast("Deleted", "info");
            emit noteCountChanged();
        }
    }
}

bool NotesManager::exportNotesAndSnippets(const QString &filePath) {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return false;

    QJsonArray arr;
    QSqlQuery q("SELECT id, title, language, code, tags, is_pinned, project_path, is_snippet, updated_at FROM notes", db);
    while (q.next()) {
        QJsonObject obj;
        obj["title"] = q.value(1).toString();
        obj["language"] = q.value(2).toString();
        obj["code"] = q.value(3).toString();
        obj["tags"] = q.value(4).toString();
        obj["is_pinned"] = (q.value(5).toInt() == 1);
        obj["project_path"] = q.value(6).toString();
        obj["is_snippet"] = (q.value(7).toInt() == 1);
        obj["updated_at"] = q.value(8).toString();
        arr.append(obj);
    }

    QJsonObject root;
    root["version"] = 1;
    root["exported_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["scratchpad"] = m_scratchpadContent;
    root["notes"] = arr;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        AppState::instance().showToast("Failed to write export file", "error");
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    AppState::instance().showToast("Exported successfully to " + filePath, "success");
    return true;
}

bool NotesManager::importNotesAndSnippets(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        AppState::instance().showToast("Failed to open import file", "error");
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        AppState::instance().showToast("Invalid JSON format", "error");
        return false;
    }

    QJsonObject root = doc.object();
    if (root.contains("scratchpad") && !root["scratchpad"].toString().isEmpty()) {
        setScratchpadContent(root["scratchpad"].toString());
    }

    QJsonArray arr = root["notes"].toArray();
    int imported = 0;
    for (const auto &val : arr) {
        QJsonObject obj = val.toObject();
        saveNote(0,
                 obj.value("title").toString(),
                 obj.value("language").toString(),
                 obj.value("code").toString(),
                 obj.value("tags").toString(),
                 obj.value("project_path").toString(),
                 obj.value("is_snippet").toBool());
        imported++;
    }

    AppState::instance().showToast(QString("Imported %1 items").arg(imported), "success");
    return true;
}

} // namespace Aura
