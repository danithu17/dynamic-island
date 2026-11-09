#include "ClipboardManager.h"
#include "../../core/DatabaseManager.h"
#include "../../core/Settings.h"
#include "../../core/AppState.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QSqlQuery>
#include <QSqlError>
#include <windows.h>
#include <QDebug>

namespace Aura {

ClipboardManager &ClipboardManager::instance() {
    static ClipboardManager s_instance;
    return s_instance;
}

ClipboardManager::ClipboardManager(QObject *parent)
    : QObject(parent) {
    loadFromDatabase();
}

void ClipboardManager::loadFromDatabase() {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery q("SELECT id, type, text_data, is_pinned, created_at FROM clipboard_items ORDER BY is_pinned DESC, id DESC LIMIT 100", db);
    std::vector<ClipboardItem> items;
    while (q.next()) {
        ClipboardItem item;
        item.id = q.value(0).toInt();
        item.type = q.value(1).toString();
        item.textData = q.value(2).toString();
        item.isPinned = (q.value(3).toInt() == 1);
        item.createdAt = q.value(4).toDateTime();
        items.push_back(std::move(item));
    }
    m_model.setItems(std::move(items));
}

bool ClipboardManager::isSensitiveContent() const {
    static UINT cfIgnore = RegisterClipboardFormatW(L"Clipboard Viewer Ignore");
    static UINT cfExclude = RegisterClipboardFormatW(L"ExcludeClipboardContentFromMonitorProcessing");

    if (cfIgnore && IsClipboardFormatAvailable(cfIgnore)) return true;
    if (cfExclude && IsClipboardFormatAvailable(cfExclude)) return true;

    return false;
}

void ClipboardManager::onNativeClipboardNotification() {
    if (!Settings::instance().clipboardEnabled()) {
        return;
    }

    if (m_internalCopy) {
        m_internalCopy = false;
        return;
    }

    if (isSensitiveContent()) {
        qDebug() << "Skipped sensitive clipboard content";
        return;
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) return;

    QString text = clipboard->text();
    if (text.trimmed().isEmpty() || text == m_lastCopiedText) {
        return;
    }
    m_lastCopiedText = text;

    ClipboardItem item;
    if (text.startsWith("http://") || text.startsWith("https://")) {
        item.type = "link";
    } else if (text.contains('{') || text.contains(';') || text.contains("def ") || text.contains("class ")) {
        item.type = "code";
    } else {
        item.type = "text";
    }

    item.textData = text;
    item.isPinned = false;
    item.createdAt = QDateTime::currentDateTime();

    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("INSERT INTO clipboard_items (type, text_data, is_pinned, created_at) VALUES (:type, :data, 0, :created)");
        q.bindValue(":type", item.type);
        q.bindValue(":data", item.textData);
        q.bindValue(":created", item.createdAt.toString(Qt::ISODate));
        if (q.exec()) {
            item.id = q.lastInsertId().toInt();
            m_model.addItem(item);
        }
    }
}

void ClipboardManager::setSearchQuery(const QString &query) {
    if (m_searchQuery != query) {
        m_searchQuery = query;
        m_model.setFilter(query);
        emit searchQueryChanged();
    }
}

void ClipboardManager::copyItem(int id) {
    for (int i = 0; i < m_model.rowCount(); ++i) {
        const ClipboardItem *item = m_model.getItemAt(i);
        if (item && item->id == id) {
            m_internalCopy = true;
            m_lastCopiedText = item->textData;
            QGuiApplication::clipboard()->setText(item->textData);
            AppState::instance().showToast("Copied to clipboard", "success");
            break;
        }
    }
}

void ClipboardManager::togglePin(int id) {
    for (int i = 0; i < m_model.rowCount(); ++i) {
        const ClipboardItem *item = m_model.getItemAt(i);
        if (item && item->id == id) {
            bool newPinned = !item->isPinned;
            auto db = DatabaseManager::instance().database();
            if (db.isOpen()) {
                QSqlQuery q(db);
                q.prepare("UPDATE clipboard_items SET is_pinned = :pinned WHERE id = :id");
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

void ClipboardManager::removeItem(int id) {
    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("DELETE FROM clipboard_items WHERE id = :id");
        q.bindValue(":id", id);
        if (q.exec()) {
            m_model.removeItemById(id);
        }
    }
}

void ClipboardManager::clearHistory() {
    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.exec("DELETE FROM clipboard_items WHERE is_pinned = 0");
        loadFromDatabase();
    }
}

} // namespace Aura
