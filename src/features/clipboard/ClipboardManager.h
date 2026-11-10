#pragma once

#include <QObject>
#include <QStringList>
#include "ClipboardModel.h"

namespace Aura {

class ClipboardManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(ClipboardModel* model READ model CONSTANT)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(QString categoryFilter READ categoryFilter WRITE setCategoryFilter NOTIFY categoryFilterChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool pasteStackActive READ pasteStackActive NOTIFY pasteStackChanged)
    Q_PROPERTY(int pasteStackCount READ pasteStackCount NOTIFY pasteStackChanged)

public:
    static ClipboardManager &instance();

    ClipboardModel *model() { return &m_model; }

    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString &query);

    QString categoryFilter() const { return m_categoryFilter; }
    Q_INVOKABLE void setCategoryFilter(const QString &category);

    int itemCount() const { return m_model.totalCount(); }
    bool pasteStackActive() const;
    int pasteStackCount() const { return static_cast<int>(m_pasteStack.size()); }

    // Core Clipboard Actions
    Q_INVOKABLE void copyItem(int id);
    Q_INVOKABLE void copyItemByIndex(int index);
    Q_INVOKABLE void togglePin(int id);
    Q_INVOKABLE void removeItem(int id);
    Q_INVOKABLE void clearHistory();

    // Programmer Quick Actions
    Q_INVOKABLE void formatJson(int id);
    Q_INVOKABLE void minifyJson(int id);
    Q_INVOKABLE void base64Encode(int id);
    Q_INVOKABLE void base64Decode(int id);
    Q_INVOKABLE void urlEncode(int id);
    Q_INVOKABLE void urlDecode(int id);
    Q_INVOKABLE void changeCase(int id, const QString &mode); // "camel", "snake", "kebab", "pascal", "upper", "lower"
    Q_INVOKABLE void trimWhitespace(int id);
    Q_INVOKABLE void wrapMarkdown(int id, const QString &lang = "");
    Q_INVOKABLE void unixTimestampConvert(int id);
    Q_INVOKABLE void generateUuid();

    // Notes Integration
    Q_INVOKABLE void sendToNotes(int id);
    Q_INVOKABLE void saveAsSnippet(int id);

    // Paste Stack & Auto-Paste
    Q_INVOKABLE void popPasteStack();
    void performAutoPaste();

public slots:
    void onNativeClipboardNotification();

signals:
    void searchQueryChanged();
    void categoryFilterChanged();
    void itemCountChanged();
    void pasteStackChanged();

private:
    explicit ClipboardManager(QObject *parent = nullptr);
    void loadFromDatabase();
    bool isSensitiveContent() const;
    bool looksLikeSecret(const QString &text) const;
    void detectTypeAndSubType(const QString &text, QString &outType, QString &outSubType) const;

    ClipboardModel m_model;
    QString m_searchQuery;
    QString m_categoryFilter = "all";
    QString m_lastCopiedText;
    bool m_internalCopy = false;

    QStringList m_pasteStack;
};

} // namespace Aura
