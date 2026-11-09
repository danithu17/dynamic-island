#include "ClipboardModel.h"

namespace Aura {

ClipboardModel::ClipboardModel(QObject *parent)
    : QAbstractListModel(parent) {
}

int ClipboardModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_filteredItems.size());
}

QVariant ClipboardModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_filteredItems.size()))
        return QVariant();

    const auto &item = m_filteredItems[index.row()];
    switch (role) {
    case IdRole: return item.id;
    case TypeRole: return item.type;
    case TextDataRole: return item.textData;
    case PreviewRole: {
        QString clean = item.textData.simplified();
        if (clean.length() > 60) return clean.left(57) + "...";
        return clean;
    }
    case IsPinnedRole: return item.isPinned;
    case CreatedAtRole: return item.createdAt.toString("hh:mm");
    default: return QVariant();
    }
}

QHash<int, QByteArray> ClipboardModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "itemId";
    roles[TypeRole] = "itemType";
    roles[TextDataRole] = "textData";
    roles[PreviewRole] = "preview";
    roles[IsPinnedRole] = "isPinned";
    roles[CreatedAtRole] = "createdAt";
    return roles;
}

void ClipboardModel::setItems(std::vector<ClipboardItem> items) {
    beginResetModel();
    m_allItems = std::move(items);
    applyFilter();
    endResetModel();
}

void ClipboardModel::addItem(const ClipboardItem &item) {
    // Avoid duplicate immediately on top
    if (!m_allItems.empty() && m_allItems.front().textData == item.textData) {
        return;
    }

    m_allItems.insert(m_allItems.begin(), item);

    // Keep max 30 unpinned items
    int unpinnedCount = 0;
    for (auto it = m_allItems.begin(); it != m_allItems.end();) {
        if (!it->isPinned) {
            unpinnedCount++;
            if (unpinnedCount > 30) {
                it = m_allItems.erase(it);
                continue;
            }
        }
        ++it;
    }

    beginResetModel();
    applyFilter();
    endResetModel();
}

void ClipboardModel::removeItemById(int id) {
    for (size_t i = 0; i < m_allItems.size(); ++i) {
        if (m_allItems[i].id == id) {
            m_allItems.erase(m_allItems.begin() + i);
            break;
        }
    }
    beginResetModel();
    applyFilter();
    endResetModel();
}

void ClipboardModel::updateItemPinned(int id, bool pinned) {
    for (auto &item : m_allItems) {
        if (item.id == id) {
            item.isPinned = pinned;
            break;
        }
    }
    beginResetModel();
    applyFilter();
    endResetModel();
}

const ClipboardItem *ClipboardModel::getItemAt(int index) const {
    if (index >= 0 && index < static_cast<int>(m_filteredItems.size())) {
        return &m_filteredItems[index];
    }
    return nullptr;
}

void ClipboardModel::setFilter(const QString &query) {
    m_filterQuery = query.trimmed().toLower();
    beginResetModel();
    applyFilter();
    endResetModel();
}

void ClipboardModel::applyFilter() {
    if (m_filterQuery.isEmpty()) {
        m_filteredItems = m_allItems;
    } else {
        m_filteredItems.clear();
        for (const auto &item : m_allItems) {
            if (item.textData.toLower().contains(m_filterQuery)) {
                m_filteredItems.push_back(item);
            }
        }
    }
}

} // namespace Aura
