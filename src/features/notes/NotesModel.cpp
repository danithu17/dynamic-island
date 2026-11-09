#include "NotesModel.h"
#include <QRegularExpression>

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
    case ProjectPathRole: return item.projectPath;
    case IsSnippetRole: return item.isSnippet;
    case UpdatedAtRole: return item.updatedAt.toString("yyyy-MM-dd hh:mm");
    case TotalTodosRole: return item.totalTodos;
    case DoneTodosRole: return item.doneTodos;
    case HasTodosRole: return (item.totalTodos > 0);
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
    roles[ProjectPathRole] = "projectPath";
    roles[IsSnippetRole] = "isSnippet";
    roles[UpdatedAtRole] = "updatedAt";
    roles[TotalTodosRole] = "totalTodos";
    roles[DoneTodosRole] = "doneTodos";
    roles[HasTodosRole] = "hasTodos";
    return roles;
}

void NotesModel::calculateTodos(NoteItem &item) const {
    item.totalTodos = 0;
    item.doneTodos = 0;

    int pos = 0;
    while ((pos = item.code.indexOf("- [", pos)) != -1) {
        if (pos + 4 <= item.code.length()) {
            QChar state = item.code[pos + 3];
            if (state == ' ' || state == 'x' || state == 'X') {
                item.totalTodos++;
                if (state == 'x' || state == 'X') item.doneTodos++;
            }
        }
        pos += 4;
    }
}

void NotesModel::setItems(std::vector<NoteItem> items) {
    beginResetModel();
    m_allItems = std::move(items);
    for (auto &item : m_allItems) {
        calculateTodos(item);
    }
    applyFilter();
    endResetModel();
    emit countChanged();
}

void NotesModel::addItem(const NoteItem &item) {
    NoteItem n = item;
    calculateTodos(n);
    m_allItems.insert(m_allItems.begin(), n);

    beginResetModel();
    applyFilter();
    endResetModel();
    emit countChanged();
}

void NotesModel::updateItem(const NoteItem &item) {
    for (auto &existing : m_allItems) {
        if (existing.id == item.id) {
            existing = item;
            calculateTodos(existing);
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
    emit countChanged();
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

const NoteItem *NotesModel::getItemById(int id) const {
    for (const auto &item : m_allItems) {
        if (item.id == id) return &item;
    }
    return nullptr;
}

void NotesModel::setFilter(const QString &query) {
    m_filterQuery = query.trimmed();
    beginResetModel();
    applyFilter();
    endResetModel();
    emit countChanged();
}

void NotesModel::setCategoryFilter(const QString &category) {
    m_category = category;
    beginResetModel();
    applyFilter();
    endResetModel();
    emit countChanged();
}

void NotesModel::setProjectFilter(const QString &projectPath) {
    m_projectFilter = projectPath;
    beginResetModel();
    applyFilter();
    endResetModel();
    emit countChanged();
}

void NotesModel::applyFilter() {
    m_filteredItems.clear();
    QString queryLower = m_filterQuery.toLower();

    for (const auto &item : m_allItems) {
        // Category check
        if (m_category == "pinned" && !item.isPinned) continue;
        if (m_category == "snippets" && !item.isSnippet) continue;
        if (m_category == "notes" && item.isSnippet) continue;

        // Project filter
        if (!m_projectFilter.isEmpty() && !item.projectPath.isEmpty() && item.projectPath != m_projectFilter) {
            continue;
        }

        // Search Query check
        if (!queryLower.isEmpty()) {
            bool matches = item.title.toLower().contains(queryLower) ||
                           item.code.toLower().contains(queryLower) ||
                           item.tags.toLower().contains(queryLower) ||
                           item.language.toLower().contains(queryLower);
            if (!matches) continue;
        }

        m_filteredItems.push_back(item);
    }
}

} // namespace Aura
