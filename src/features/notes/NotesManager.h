#pragma once

#include <QObject>
#include "NotesModel.h"

namespace Aura {

class NotesManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(NotesModel* model READ model CONSTANT)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)

public:
    static NotesManager &instance();

    NotesModel *model() { return &m_model; }

    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString &query);

    Q_INVOKABLE void saveNote(int id, const QString &title, const QString &language, const QString &code, const QString &tags);
    Q_INVOKABLE void copyCode(int id);
    Q_INVOKABLE void togglePin(int id);
    Q_INVOKABLE void deleteNote(int id);

signals:
    void searchQueryChanged();

private:
    explicit NotesManager(QObject *parent = nullptr);
    void loadFromDatabase();

    NotesModel m_model;
    QString m_searchQuery;
};

} // namespace Aura
