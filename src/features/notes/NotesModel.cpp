#include "NotesModel.h"

namespace Aura {

NotesModel::NotesModel(QObject *parent)
    : QAbstractListModel(parent) {
}

int NotesModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_filteredItems.size());
}

QVariant NotesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_filteredItems.size()))
        return QVariant();

    const auto &item = m_filteredItems[index.row()];
    switch (role) {
    case IdRole: return item.id;
    case TitleRole: return item.title;
    case LanguageRole: return item.language;
    case CodeRole: return item.code;
    case TagsRole: return item.tags;
    case IsPinnedRole: return item.isPinned;
    case UpdatedAtRole: return item.updatedAt.toString("MMM d, hh:mm");
    default: return QVariant();
    }
}

QHash<int, QByteArray> NotesModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "noteId";
    roles[TitleRole] = "title";
    roles[LanguageRole] = "language";
    roles[CodeRole] = "code";
    roles[TagsRole] = "tags";
    roles[IsPinnedRole] = "isPinned";
    roles[UpdatedAtRole] = "updatedAt";
    return roles;
}

void NotesModel::setItems(std::vector<NoteItem> items) {
    beginResetModel();
    m_allItems = std::move(items);
    applyFilter();
    endResetModel();
}

void NotesModel::addItem(const NoteItem &item) {
    m_allItems.insert(m_allItems.begin(), item);
    beginResetModel();
    applyFilter();
    endResetModel();
}

void NotesModel::updateItem(const NoteItem &item) {
    for (auto &n : m_allItems) {
        if (n.id == item.id) {
            n = item;
            break;
        }
    }
    beginResetModel();
    applyFilter();
    endResetModel();
}

void NotesModel::removeItemById(int id) {
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

void NotesModel::updateItemPinned(int id, bool pinned) {
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

const NoteItem *NotesModel::getItemAt(int index) const {
    if (index >= 0 && index < static_cast<int>(m_filteredItems.size())) {
        return &m_filteredItems[index];
    }
    return nullptr;
}

void NotesModel::setFilter(const QString &query) {
    m_filterQuery = query.trimmed().toLower();
    beginResetModel();
    applyFilter();
    endResetModel();
}

void NotesModel::applyFilter() {
    if (m_filterQuery.isEmpty()) {
        m_filteredItems = m_allItems;
    } else {
        m_filteredItems.clear();
        for (const auto &item : m_allItems) {
            if (item.title.toLower().contains(m_filterQuery) ||
                item.code.toLower().contains(m_filterQuery) ||
                item.tags.toLower().contains(m_filterQuery) ||
                item.language.toLower().contains(m_filterQuery)) {
                m_filteredItems.push_back(item);
            }
        }
    }
}

} // namespace Aura
