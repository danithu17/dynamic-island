#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <vector>

namespace Aura {

struct ClipboardItem {
    int id = 0;
    QString type = "text";       // "code", "url", "json", "hex_color", "file_path", "shell_command", "stack_trace", "text", "image"
    QString subType = "plaintext"; // language guess or format
    QString textData;
    bool isPinned = false;
    QDateTime createdAt;
    int lineCount = 1;
    int charCount = 0;
};

class ClipboardModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum ClipboardRoles {
        IdRole = Qt::UserRole + 1,
        TypeRole,
        SubTypeRole,
        TextDataRole,
        PreviewRole,
        IsPinnedRole,
        CreatedAtRole,
        LineCountRole,
        CharCountRole
    };

    explicit ClipboardModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return static_cast<int>(m_filteredItems.size()); }
    int totalCount() const { return static_cast<int>(m_allItems.size()); }

    void setItems(std::vector<ClipboardItem> items);
    void addItem(const ClipboardItem &item);
    void removeItemById(int id);
    void updateItemPinned(int id, bool pinned);
    void updateItemText(int id, const QString &newText, const QString &newType);
    const ClipboardItem *getItemAt(int index) const;
    const ClipboardItem *getItemById(int id) const;

    void setFilter(const QString &query);
    void setCategoryFilter(const QString &category);

signals:
    void countChanged();

private:
    std::vector<ClipboardItem> m_allItems;
    std::vector<ClipboardItem> m_filteredItems;
    QString m_filterQuery;
    QString m_category = "all"; // "all", "code", "links", "images", "pinned"

    void applyFilter();
};

} // namespace Aura
