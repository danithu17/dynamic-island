#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <vector>

namespace Aura {

struct ClipboardItem {
    int id = 0;
    QString type; // "text", "link", "code"
    QString textData;
    bool isPinned = false;
    QDateTime createdAt;
};

class ClipboardModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum ClipboardRoles {
        IdRole = Qt::UserRole + 1,
        TypeRole,
        TextDataRole,
        PreviewRole,
        IsPinnedRole,
        CreatedAtRole
    };

    explicit ClipboardModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(std::vector<ClipboardItem> items);
    void addItem(const ClipboardItem &item);
    void removeItemById(int id);
    void updateItemPinned(int id, bool pinned);
    const ClipboardItem *getItemAt(int index) const;

    void setFilter(const QString &query);

private:
    std::vector<ClipboardItem> m_allItems;
    std::vector<ClipboardItem> m_filteredItems;
    QString m_filterQuery;

    void applyFilter();
};

} // namespace Aura
