#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>

namespace Aura {

struct GmailMessage {
    QString id;
    QString threadId;
    QString sender;
    QString subject;
    QString snippet;
    QString date;
    bool isUnread = true;
};

class GmailModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum GmailRoles {
        IdRole = Qt::UserRole + 1,
        ThreadIdRole,
        SenderRole,
        SubjectRole,
        SnippetRole,
        DateRole,
        IsUnreadRole
    };

    explicit GmailModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setMessages(std::vector<GmailMessage> messages);
    void removeMessageById(const QString &id);
    const GmailMessage *getMessageAt(int index) const;

private:
    std::vector<GmailMessage> m_messages;
};

} // namespace Aura
