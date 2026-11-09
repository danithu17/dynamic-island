#pragma once

#include <QObject>
#include "ClipboardModel.h"

namespace Aura {

class ClipboardManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(ClipboardModel* model READ model CONSTANT)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)

public:
    static ClipboardManager &instance();

    ClipboardModel *model() { return &m_model; }

    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString &query);

    Q_INVOKABLE void copyItem(int id);
    Q_INVOKABLE void togglePin(int id);
    Q_INVOKABLE void removeItem(int id);
    Q_INVOKABLE void clearHistory();

public slots:
    void onNativeClipboardNotification();

signals:
    void searchQueryChanged();

private:
    explicit ClipboardManager(QObject *parent = nullptr);
    void loadFromDatabase();
    bool isSensitiveContent() const;

    ClipboardModel m_model;
    QString m_searchQuery;
    QString m_lastCopiedText;
    bool m_internalCopy = false;
};

} // namespace Aura
