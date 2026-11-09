#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <vector>

namespace Aura {

struct ShelfItem {
    int id = 0;
    QString type;      // "file", "image", "link", "text"
    QString title;
    QString filePath;
    QString data;
    bool isPinned = false;
    QDateTime createdAt;
};

class ShelfModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum ShelfRoles {
        IdRole = Qt::UserRole + 1,
        TypeRole,
        TitleRole,
        FilePathRole,
        DataRole,
        IsPinnedRole,
        CreatedAtRole
    };

    explicit ShelfModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(std::vector<ShelfItem> items);
    void addItem(const ShelfItem &item);
    void removeItemById(int id);
    void updateItemPinned(int id, bool pinned);
    const ShelfItem *getItemAt(int index) const;

private:
    std::vector<ShelfItem> m_items;
};

} // namespace Aura
