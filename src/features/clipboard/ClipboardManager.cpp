#include "ClipboardManager.h"
#include "../../core/DatabaseManager.h"
#include "../../core/Settings.h"
#include "../../core/AppState.h"
#include "../notes/NotesManager.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QRegularExpression>
#include <QUuid>
#include <QFileInfo>
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

bool ClipboardManager::pasteStackActive() const {
    return Settings::instance().pasteStackEnabled();
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
        item.charCount = item.textData.length();
        item.lineCount = item.textData.count('\n') + 1;

        // Extract subType
        QString dummyType;
        detectTypeAndSubType(item.textData, dummyType, item.subType);

        items.push_back(std::move(item));
    }
    m_model.setItems(std::move(items));
    emit itemCountChanged();
}

bool ClipboardManager::isSensitiveContent() const {
    static UINT cfIgnore = RegisterClipboardFormatW(L"Clipboard Viewer Ignore");
    static UINT cfExclude = RegisterClipboardFormatW(L"ExcludeClipboardContentFromMonitorProcessing");
    static UINT cfCanInclude = RegisterClipboardFormatW(L"CanIncludeInClipboardHistory");

    if (cfIgnore && IsClipboardFormatAvailable(cfIgnore)) return true;
    if (cfExclude && IsClipboardFormatAvailable(cfExclude)) return true;
    if (cfCanInclude && !IsClipboardFormatAvailable(cfCanInclude)) return true;

    return false;
}

bool ClipboardManager::looksLikeSecret(const QString &text) const {
    if (text.length() < 16) return false;

    // Common secret patterns
    static const QRegularExpression patterns[] = {
        QRegularExpression(R"((?i)(api[_-]?key|secret|token|password|auth|bearer)\s*[:=]\s*['"]?[a-zA-Z0-9_\-\.]{12,})"),
        QRegularExpression(R"(sk-[a-zA-Z0-9]{20,})"),                         // OpenAI
        QRegularExpression(R"(ghp_[a-zA-Z0-9]{36})"),                          // GitHub PAT
        QRegularExpression(R"(github_pat_[a-zA-Z0-9_]{30,})"),                 // Fine-grained PAT
        QRegularExpression(R"(AKIA[0-9A-Z]{16})"),                             // AWS Access Key
        QRegularExpression(R"(eyJ[a-zA-Z0-9_\-]{10,}\.[a-zA-Z0-9_\-]{10,}\.[a-zA-Z0-9_\-]{10,})"), // JWT
        QRegularExpression(R"(-----BEGIN [A-Z ]*PRIVATE KEY-----)"),          // PEM private key
        QRegularExpression(R"(AIza[0-9A-Za-z\-_]{35})")                        // Google API key
    };

    for (const auto &regex : patterns) {
        if (regex.match(text).hasMatch()) {
            return true;
        }
    }

    return false;
}

void ClipboardManager::detectTypeAndSubType(const QString &text, QString &outType, QString &outSubType) const {
    QString trimmed = text.trimmed();

    // 1. JSON
    if ((trimmed.startsWith('{') && trimmed.endsWith('}')) || (trimmed.startsWith('[') && trimmed.endsWith(']'))) {
        QJsonParseError err;
        QJsonDocument::fromJson(trimmed.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError) {
            outType = "json";
            outSubType = "json";
            return;
        }
    }

    // 2. Hex color
    static const QRegularExpression hexRegex(R"(^#([0-9a-fA-F]{3}|[0-9a-fA-F]{6}|[0-9a-fA-F]{8})$)");
    if (hexRegex.match(trimmed).hasMatch()) {
        outType = "hex_color";
        outSubType = "color";
        return;
    }

    // 3. URL
    if (trimmed.startsWith("http://", Qt::CaseInsensitive) || trimmed.startsWith("https://", Qt::CaseInsensitive)) {
        QUrl url(trimmed);
        if (url.isValid()) {
            outType = "url";
            outSubType = "link";
            return;
        }
    }

    // 4. Shell commands
    static const QStringList shellPrefixes = {
        "git ", "npm ", "pnpm ", "yarn ", "cargo ", "pip ", "docker ", "kubectl ",
        "curl ", "cd ", "mkdir ", "chmod ", "powershell ", "cmake ", "dotnet "
    };
    for (const auto &pfx : shellPrefixes) {
        if (trimmed.startsWith(pfx, Qt::CaseInsensitive)) {
            outType = "shell_command";
            outSubType = "shell";
            return;
        }
    }

    // 5. File Path
    if (trimmed.length() < 260 && (trimmed.contains('/') || trimmed.contains('\\'))) {
        if (trimmed.contains(":\\") || trimmed.startsWith('/') || trimmed.startsWith("./") || trimmed.startsWith("~/")) {
            outType = "file_path";
            outSubType = QFileInfo(trimmed).suffix();
            return;
        }
    }

    // 6. Stack Trace
    if (trimmed.contains("Traceback (most recent call last)") || trimmed.contains("   at ") || trimmed.contains("Exception in thread")) {
        outType = "stack_trace";
        outSubType = "log";
        return;
    }

    // 7. Code (detect common languages)
    if (trimmed.contains('{') || trimmed.contains(';') || trimmed.contains("=>") ||
        trimmed.contains("def ") || trimmed.contains("class ") || trimmed.contains("import ") ||
        trimmed.contains("#include") || trimmed.contains("func ")) {
        outType = "code";
        if (trimmed.contains("#include") || trimmed.contains("std::")) outSubType = "cpp";
        else if (trimmed.contains("def ") || trimmed.contains("import ") && trimmed.contains(":")) outSubType = "python";
        else if (trimmed.contains("const ") || trimmed.contains("let ") || trimmed.contains("function")) outSubType = "javascript";
        else if (trimmed.contains("package ") || trimmed.contains("func ")) outSubType = "go";
        else if (trimmed.contains("fn ") || trimmed.contains("let mut")) outSubType = "rust";
        else outSubType = "code";
        return;
    }

    outType = "text";
    outSubType = "plaintext";
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
        qDebug() << "Skipped sensitive clipboard content (system flag)";
        return;
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) return;

    QString text = clipboard->text();
    if (text.trimmed().isEmpty() || text == m_lastCopiedText) {
        return;
    }

    // Check secrets
    if (looksLikeSecret(text)) {
        qDebug() << "Skipped sensitive clipboard content (secret pattern)";
        AppState::instance().showToast("Ignored sensitive token/secret from history", "warning");
        return;
    }

    m_lastCopiedText = text;

    ClipboardItem item;
    detectTypeAndSubType(text, item.type, item.subType);
    item.textData = text;
    item.isPinned = false;
    item.createdAt = QDateTime::currentDateTime();
    item.charCount = text.length();
    item.lineCount = text.count('\n') + 1;

    // Paste Stack queue
    if (Settings::instance().pasteStackEnabled()) {
        m_pasteStack.push_back(text);
        emit pasteStackChanged();
    }

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
            emit itemCountChanged();
        }
    }

    // Event morph: compact for 3 seconds
    QString preview = text.simplified();
    if (preview.length() > 32) preview = preview.left(30) + "...";
    AppState::instance().triggerEventCompact("Copied: " + preview, 3000);
}

void ClipboardManager::setSearchQuery(const QString &query) {
    if (m_searchQuery != query) {
        m_searchQuery = query;
        m_model.setFilter(query);
        emit searchQueryChanged();
    }
}

void ClipboardManager::setCategoryFilter(const QString &category) {
    if (m_categoryFilter != category) {
        m_categoryFilter = category;
        m_model.setCategoryFilter(category);
        emit categoryFilterChanged();
    }
}

void ClipboardManager::copyItem(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    m_internalCopy = true;
    m_lastCopiedText = item->textData;
    QGuiApplication::clipboard()->setText(item->textData);
    AppState::instance().showToast("Copied to clipboard", "success");

    if (Settings::instance().autoPaste()) {
        performAutoPaste();
    }
}

void ClipboardManager::copyItemByIndex(int index) {
    const ClipboardItem *item = m_model.getItemAt(index);
    if (item) {
        copyItem(item->id);
    }
}

void ClipboardManager::performAutoPaste() {
    // Synthesize Ctrl+V to active window via SendInput
    INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
}

void ClipboardManager::popPasteStack() {
    if (m_pasteStack.isEmpty()) {
        AppState::instance().showToast("Paste stack is empty", "info");
        return;
    }

    QString nextText = m_pasteStack.takeFirst();
    emit pasteStackChanged();

    m_internalCopy = true;
    m_lastCopiedText = nextText;
    QGuiApplication::clipboard()->setText(nextText);

    QString preview = nextText.simplified();
    if (preview.length() > 24) preview = preview.left(22) + "...";
    AppState::instance().showToast("Pasted from stack (" + QString::number(m_pasteStack.size()) + " remaining): " + preview, "success");

    if (Settings::instance().autoPaste()) {
        performAutoPaste();
    }
}

void ClipboardManager::togglePin(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    bool newPinned = !item->isPinned;
    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("UPDATE clipboard_items SET is_pinned = :pinned WHERE id = :id");
        q.bindValue(":pinned", newPinned ? 1 : 0);
        q.bindValue(":id", id);
        if (q.exec()) {
            m_model.updateItemPinned(id, newPinned);
            AppState::instance().showToast(newPinned ? "Pinned" : "Unpinned", "info");
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
            emit itemCountChanged();
        }
    }
}

void ClipboardManager::clearHistory() {
    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.exec("DELETE FROM clipboard_items WHERE is_pinned = 0");
        loadFromDatabase();
        AppState::instance().showToast("Cleared unpinned clipboard history", "info");
    }
}

// ─────────────────────────────────────────────────────────────
// Programmer Quick Actions
// ─────────────────────────────────────────────────────────────
void ClipboardManager::formatJson(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(item->textData.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError) {
        QString formatted = QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
        m_model.updateItemText(id, formatted, "json");
        copyItem(id);
        AppState::instance().showToast("JSON formatted & copied", "success");
    } else {
        AppState::instance().showToast("Invalid JSON: " + err.errorString(), "error");
    }
}

void ClipboardManager::minifyJson(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(item->textData.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError) {
        QString minified = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        m_model.updateItemText(id, minified, "json");
        copyItem(id);
        AppState::instance().showToast("JSON minified & copied", "success");
    } else {
        AppState::instance().showToast("Invalid JSON", "error");
    }
}

void ClipboardManager::base64Encode(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QString enc = QString::fromUtf8(item->textData.toUtf8().toBase64());
    m_model.updateItemText(id, enc, "text");
    copyItem(id);
    AppState::instance().showToast("Base64 encoded & copied", "success");
}

void ClipboardManager::base64Decode(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QByteArray decBytes = QByteArray::fromBase64(item->textData.toUtf8());
    QString dec = QString::fromUtf8(decBytes);
    m_model.updateItemText(id, dec, "text");
    copyItem(id);
    AppState::instance().showToast("Base64 decoded & copied", "success");
}

void ClipboardManager::urlEncode(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QString enc = QString::fromUtf8(QUrl::toPercentEncoding(item->textData));
    m_model.updateItemText(id, enc, "text");
    copyItem(id);
    AppState::instance().showToast("URL encoded & copied", "success");
}

void ClipboardManager::urlDecode(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QString dec = QUrl::fromPercentEncoding(item->textData.toUtf8());
    m_model.updateItemText(id, dec, "text");
    copyItem(id);
    AppState::instance().showToast("URL decoded & copied", "success");
}

void ClipboardManager::changeCase(int id, const QString &mode) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QString original = item->textData;
    QString result;

    if (mode == "upper") {
        result = original.toUpper();
    } else if (mode == "lower") {
        result = original.toLower();
    } else {
        // Tokenize words
        QStringList words = original.split(QRegularExpression(R"([\s_\-]+)"), Qt::SkipEmptyParts);
        if (words.isEmpty()) return;

        if (mode == "camel") {
            for (int i = 0; i < words.size(); ++i) {
                QString w = words[i].toLower();
                if (i > 0 && !w.isEmpty()) w[0] = w[0].toUpper();
                result += w;
            }
        } else if (mode == "pascal") {
            for (const auto &word : words) {
                QString w = word.toLower();
                if (!w.isEmpty()) w[0] = w[0].toUpper();
                result += w;
            }
        } else if (mode == "snake") {
            for (int i = 0; i < words.size(); ++i) {
                if (i > 0) result += "_";
                result += words[i].toLower();
            }
        } else if (mode == "kebab") {
            for (int i = 0; i < words.size(); ++i) {
                if (i > 0) result += "-";
                result += words[i].toLower();
            }
        }
    }

    if (!result.isEmpty()) {
        m_model.updateItemText(id, result, item->type);
        copyItem(id);
        AppState::instance().showToast("Converted to " + mode + " case", "success");
    }
}

void ClipboardManager::trimWhitespace(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QString trimmed = item->textData.trimmed();
    m_model.updateItemText(id, trimmed, item->type);
    copyItem(id);
    AppState::instance().showToast("Trimmed whitespace", "success");
}

void ClipboardManager::wrapMarkdown(int id, const QString &lang) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QString l = lang.isEmpty() ? (item->subType.isEmpty() ? "" : item->subType) : lang;
    QString wrapped = "```" + l + "\n" + item->textData + "\n```";
    m_model.updateItemText(id, wrapped, "code");
    copyItem(id);
    AppState::instance().showToast("Wrapped in Markdown code block", "success");
}

void ClipboardManager::unixTimestampConvert(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QString text = item->textData.trimmed();
    bool ok = false;
    qint64 ts = text.toLongLong(&ok);

    if (ok) {
        // If length > 11, likely milliseconds
        if (text.length() > 11) ts /= 1000;
        QDateTime dt = QDateTime::fromSecsSinceEpoch(ts, QTimeZone::UTC);
        QString dateStr = dt.toString(Qt::ISODate) + " UTC";
        m_model.updateItemText(id, dateStr, "text");
        copyItem(id);
        AppState::instance().showToast("Converted to Date: " + dateStr, "success");
    } else {
        // Try parsing ISO date to unix timestamp
        QDateTime dt = QDateTime::fromString(text, Qt::ISODate);
        if (dt.isValid()) {
            QString tsStr = QString::number(dt.toSecsSinceEpoch());
            m_model.updateItemText(id, tsStr, "text");
            copyItem(id);
            AppState::instance().showToast("Converted to Unix timestamp: " + tsStr, "success");
        } else {
            AppState::instance().showToast("Could not parse timestamp or date", "warning");
        }
    }
}

void ClipboardManager::generateUuid() {
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_internalCopy = true;
    m_lastCopiedText = uuid;
    QGuiApplication::clipboard()->setText(uuid);
    AppState::instance().showToast("Generated UUID: " + uuid, "success");

    ClipboardItem item;
    item.type = "text";
    item.subType = "uuid";
    item.textData = uuid;
    item.isPinned = false;
    item.createdAt = QDateTime::currentDateTime();
    item.charCount = uuid.length();
    item.lineCount = 1;
    m_model.addItem(item);
    emit itemCountChanged();
}

void ClipboardManager::sendToNotes(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QString title = item->textData.trimmed().section('\n', 0, 0);
    if (title.length() > 40) title = title.left(37) + "...";
    if (title.isEmpty()) title = "Clipboard Note";

    NotesManager::instance().saveNote(0, title, item->subType, item->textData, "clipboard", "", false);
    AppState::instance().showToast("Saved to Notes", "success");
}

void ClipboardManager::saveAsSnippet(int id) {
    const ClipboardItem *item = m_model.getItemById(id);
    if (!item) return;

    QString title = "Snippet: " + item->textData.trimmed().section('\n', 0, 0);
    if (title.length() > 35) title = title.left(32) + "...";

    NotesManager::instance().saveNote(0, title, item->subType, item->textData, "snippet", "", true);
    AppState::instance().showToast("Saved as Snippet", "success");
}

} // namespace Aura
