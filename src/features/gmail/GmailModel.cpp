#include "GmailModel.h"

namespace Aura {

GmailModel::GmailModel(QObject *parent)
    : QAbstractListModel(parent) {
}

int GmailModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_messages.size());
}

QVariant GmailModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_messages.size()))
        return QVariant();

    const auto &msg = m_messages[index.row()];
    switch (role) {
    case IdRole: return msg.id;
    case ThreadIdRole: return msg.threadId;
    case SenderRole: return msg.sender;
    case SubjectRole: return msg.subject;
    case SnippetRole: return msg.snippet;
    case DateRole: return msg.date;
    case IsUnreadRole: return msg.isUnread;
    default: return QVariant();
    }
}

QHash<int, QByteArray> GmailModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "messageId";
    roles[ThreadIdRole] = "threadId";
    roles[SenderRole] = "sender";
    roles[SubjectRole] = "subject";
    roles[SnippetRole] = "snippet";
    roles[DateRole] = "date";
    roles[IsUnreadRole] = "isUnread";
    return roles;
}

void GmailModel::setMessages(std::vector<GmailMessage> messages) {
    beginResetModel();
    m_messages = std::move(messages);
    endResetModel();
}

void GmailModel::removeMessageById(const QString &id) {
    for (size_t i = 0; i < m_messages.size(); ++i) {
        if (m_messages[i].id == id) {
            beginRemoveRows(QModelIndex(), static_cast<int>(i), static_cast<int>(i));
            m_messages.erase(m_messages.begin() + i);
            endRemoveRows();
            break;
        }
    }
}

const GmailMessage *GmailModel::getMessageAt(int index) const {
    if (index >= 0 && index < static_cast<int>(m_messages.size())) {
        return &m_messages[index];
    }
    return nullptr;
}

} // namespace Aura
