#include "ShelfModel.h"

namespace Aura {

ShelfModel::ShelfModel(QObject *parent)
    : QAbstractListModel(parent) {
}

int ShelfModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_items.size());
}

QVariant ShelfModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_items.size()))
        return QVariant();

    const auto &item = m_items[index.row()];
    switch (role) {
    case IdRole: return item.id;
    case TypeRole: return item.type;
    case TitleRole: return item.title;
    case FilePathRole: return item.filePath;
    case DataRole: return item.data;
    case IsPinnedRole: return item.isPinned;
    case CreatedAtRole: return item.createdAt.toString("MMM d, hh:mm");
    default: return QVariant();
    }
}

QHash<int, QByteArray> ShelfModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "itemId";
    roles[TypeRole] = "itemType";
    roles[TitleRole] = "title";
    roles[FilePathRole] = "filePath";
    roles[DataRole] = "itemData";
    roles[IsPinnedRole] = "isPinned";
    roles[CreatedAtRole] = "createdAt";
    return roles;
}

void ShelfModel::setItems(std::vector<ShelfItem> items) {
    beginResetModel();
    m_items = std::move(items);
    endResetModel();
}

void ShelfModel::addItem(const ShelfItem &item) {
    beginInsertRows(QModelIndex(), 0, 0);
    m_items.insert(m_items.begin(), item);
    endInsertRows();
}

void ShelfModel::removeItemById(int id) {
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            beginRemoveRows(QModelIndex(), static_cast<int>(i), static_cast<int>(i));
            m_items.erase(m_items.begin() + i);
            endRemoveRows();
            break;
        }
    }
}

void ShelfModel::updateItemPinned(int id, bool pinned) {
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            m_items[i].isPinned = pinned;
            QModelIndex idx = index(static_cast<int>(i), 0);
            emit dataChanged(idx, idx, {IsPinnedRole});
            break;
        }
    }
}

const ShelfItem *ShelfModel::getItemAt(int index) const {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        return &m_items[index];
    }
    return nullptr;
}

} // namespace Aura
