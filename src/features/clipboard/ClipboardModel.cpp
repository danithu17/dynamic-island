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
    case SubTypeRole: return item.subType;
    case TextDataRole: return item.textData;
    case PreviewRole: {
        QString clean = item.textData.simplified();
        if (clean.length() > 65) return clean.left(62) + "...";
        return clean;
    }
    case IsPinnedRole: return item.isPinned;
    case CreatedAtRole: return item.createdAt.toString("hh:mm");
    case LineCountRole: return item.lineCount;
    case CharCountRole: return item.charCount;
    default: return QVariant();
    }
}

QHash<int, QByteArray> ClipboardModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "itemId";
    roles[TypeRole] = "itemType";
    roles[SubTypeRole] = "itemSubType";
    roles[TextDataRole] = "textData";
    roles[PreviewRole] = "preview";
    roles[IsPinnedRole] = "isPinned";
    roles[CreatedAtRole] = "createdAt";
    roles[LineCountRole] = "lineCount";
    roles[CharCountRole] = "charCount";
    return roles;
}

void ClipboardModel::setItems(std::vector<ClipboardItem> items) {
    beginResetModel();
    m_allItems = std::move(items);
    applyFilter();
    endResetModel();
    emit countChanged();
}

void ClipboardModel::addItem(const ClipboardItem &item) {
    // Avoid duplicate immediately on top
    if (!m_allItems.empty() && m_allItems.front().textData == item.textData) {
        return;
    }

    m_allItems.insert(m_allItems.begin(), item);

    // Keep max 100 unpinned items (pinned items never expire)
    int unpinnedCount = 0;
    for (auto it = m_allItems.begin(); it != m_allItems.end();) {
        if (!it->isPinned) {
            unpinnedCount++;
            if (unpinnedCount > 100) {
                it = m_allItems.erase(it);
                continue;
            }
        }
        ++it;
    }

    beginResetModel();
    applyFilter();
    endResetModel();
    emit countChanged();
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
    emit countChanged();
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

void ClipboardModel::updateItemText(int id, const QString &newText, const QString &newType) {
    for (auto &item : m_allItems) {
        if (item.id == id) {
            item.textData = newText;
            if (!newType.isEmpty()) item.type = newType;
            item.charCount = newText.length();
            item.lineCount = newText.count('\n') + 1;
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

const ClipboardItem *ClipboardModel::getItemById(int id) const {
    for (const auto &item : m_allItems) {
        if (item.id == id) return &item;
    }
    return nullptr;
}

void ClipboardModel::setFilter(const QString &query) {
    m_filterQuery = query.trimmed();
    beginResetModel();
    applyFilter();
    endResetModel();
    emit countChanged();
}

void ClipboardModel::setCategoryFilter(const QString &category) {
    m_category = category;
    beginResetModel();
    applyFilter();
    endResetModel();
    emit countChanged();
}

void ClipboardModel::applyFilter() {
    m_filteredItems.clear();
    QString queryLower = m_filterQuery.toLower();

    for (const auto &item : m_allItems) {
        // Category check
        if (m_category == "pinned" && !item.isPinned) continue;
        if (m_category == "code" && item.type != "code" && item.type != "json" && item.type != "shell_command") continue;
        if (m_category == "links" && item.type != "url") continue;
        if (m_category == "images" && item.type != "image") continue;

        // Query check
        if (!queryLower.isEmpty()) {
            bool matches = item.textData.toLower().contains(queryLower) ||
                           item.type.toLower().contains(queryLower) ||
                           item.subType.toLower().contains(queryLower);
            if (!matches) continue;
        }

        m_filteredItems.push_back(item);
    }
}

} // namespace Aura
