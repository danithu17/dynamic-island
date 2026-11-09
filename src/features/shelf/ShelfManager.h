#pragma once

#include <QObject>
#include <QStringList>
#include "ShelfModel.h"

namespace Aura {

class ShelfManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(ShelfModel* model READ model CONSTANT)

public:
    static ShelfManager &instance();

    ShelfModel *model() { return &m_model; }

    Q_INVOKABLE void addFile(const QString &localFilePath);
    Q_INVOKABLE void addDropUrls(const QList<QUrl> &urls);
    Q_INVOKABLE void addText(const QString &text);
    Q_INVOKABLE void openItem(int id);
    Q_INVOKABLE void copyPath(int id);
    Q_INVOKABLE void togglePin(int id);
    Q_INVOKABLE void removeItem(int id);
    Q_INVOKABLE void clearUnpinned();

    void loadFromDatabase();

private:
    explicit ShelfManager(QObject *parent = nullptr);

    ShelfModel m_model;
};

} // namespace Aura
