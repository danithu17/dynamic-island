#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <vector>

namespace Aura {

struct NoteItem {
    int id = 0;
    QString title;
    QString language = "plaintext";
    QString code;
    QString tags;
    bool isPinned = false;
    QDateTime updatedAt;
};

class NotesModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum NoteRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        LanguageRole,
        CodeRole,
        TagsRole,
        IsPinnedRole,
        UpdatedAtRole
    };

    explicit NotesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(std::vector<NoteItem> items);
    void addItem(const NoteItem &item);
    void updateItem(const NoteItem &item);
    void removeItemById(int id);
    void updateItemPinned(int id, bool pinned);
    const NoteItem *getItemAt(int index) const;

    void setFilter(const QString &query);

private:
    std::vector<NoteItem> m_allItems;
    std::vector<NoteItem> m_filteredItems;
    QString m_filterQuery;

    void applyFilter();
};

} // namespace Aura
