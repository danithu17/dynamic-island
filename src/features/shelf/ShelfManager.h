#pragma once

#include <QObject>
#include <QStringList>
#include "ShelfModel.h"

namespace Aura {

class ShelfManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(ShelfModel* model READ model CONSTANT)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)

public:
    static ShelfManager &instance();

    ShelfModel *model() { return &m_model; }
    int itemCount() const { return m_model.rowCount(); }

    Q_INVOKABLE void addFile(const QString &localFilePath);
    Q_INVOKABLE void addDropUrls(const QList<QUrl> &urls);
    Q_INVOKABLE void addText(const QString &text);
    Q_INVOKABLE void openItem(int id);
    Q_INVOKABLE void copyPath(int id);
    Q_INVOKABLE void togglePin(int id);
    Q_INVOKABLE void removeItem(int id);
    Q_INVOKABLE void clearUnpinned();
    Q_INVOKABLE void revealInExplorer(int id);
    Q_INVOKABLE void openTrayFolder();
    Q_INVOKABLE void setSearchQuery(const QString &query);

    void loadFromDatabase();

signals:
    void itemCountChanged();

private:
    explicit ShelfManager(QObject *parent = nullptr);

    ShelfModel m_model;
};

} // namespace Aura
