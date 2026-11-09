#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <vector>

namespace Aura {

struct NoteItem {
    int id = 0;
    QString title;
    QString language = "markdown";
    QString code;
    QString tags;
    bool isPinned = false;
    QString projectPath = "";
    bool isSnippet = false;
    QDateTime updatedAt;

    int totalTodos = 0;
    int doneTodos = 0;
};

class NotesModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum NoteRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        LanguageRole,
        CodeRole,
        TagsRole,
        IsPinnedRole,
        ProjectPathRole,
        IsSnippetRole,
        UpdatedAtRole,
        TotalTodosRole,
        DoneTodosRole,
        HasTodosRole
    };

    explicit NotesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return static_cast<int>(m_filteredItems.size()); }
    int totalCount() const { return static_cast<int>(m_allItems.size()); }

    void setItems(std::vector<NoteItem> items);
    void addItem(const NoteItem &item);
    void updateItem(const NoteItem &item);
    void removeItemById(int id);
    void updateItemPinned(int id, bool pinned);
    const NoteItem *getItemAt(int index) const;
    const NoteItem *getItemById(int id) const;

    void setFilter(const QString &query);
    void setCategoryFilter(const QString &category);
    void setProjectFilter(const QString &projectPath);

signals:
    void countChanged();

private:
    std::vector<NoteItem> m_allItems;
    std::vector<NoteItem> m_filteredItems;
    QString m_filterQuery;
    QString m_category = "all"; // "all", "notes", "snippets", "pinned"
    QString m_projectFilter = "";

    void calculateTodos(NoteItem &item) const;
    void applyFilter();
};

} // namespace Aura
