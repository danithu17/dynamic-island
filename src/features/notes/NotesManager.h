#pragma once

#include <QObject>
#include "NotesModel.h"

namespace Aura {

class NotesManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(NotesModel* model READ model CONSTANT)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(QString categoryFilter READ categoryFilter WRITE setCategoryFilter NOTIFY categoryFilterChanged)
    Q_PROPERTY(QString scratchpadContent READ scratchpadContent WRITE setScratchpadContent NOTIFY scratchpadContentChanged)
    Q_PROPERTY(QString activeProject READ activeProject WRITE setActiveProject NOTIFY activeProjectChanged)
    Q_PROPERTY(int noteCount READ noteCount NOTIFY noteCountChanged)

public:
    static NotesManager &instance();

    NotesModel *model() { return &m_model; }

    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString &query);

    QString categoryFilter() const { return m_categoryFilter; }
    void setCategoryFilter(const QString &category);

    QString scratchpadContent() const { return m_scratchpadContent; }
    void setScratchpadContent(const QString &text);

    QString activeProject() const { return m_activeProject; }
    void setActiveProject(const QString &proj);

    int noteCount() const { return m_model.totalCount(); }

    Q_INVOKABLE void saveNote(int id, const QString &title, const QString &language,
                              const QString &code, const QString &tags,
                              const QString &projectPath = "", bool isSnippet = false);
    Q_INVOKABLE void copyCode(int id);
    Q_INVOKABLE void copySnippetWithVariables(int id);
    Q_INVOKABLE void toggleTodoCheckbox(int id, int todoIndex);
    Q_INVOKABLE void quickCapture(const QString &text);
    Q_INVOKABLE void togglePin(int id);
    Q_INVOKABLE void deleteNote(int id);
    Q_INVOKABLE void clearScratchpad();

    // Import / Export (100% local)
    Q_INVOKABLE bool exportNotesAndSnippets(const QString &filePath);
    Q_INVOKABLE bool importNotesAndSnippets(const QString &filePath);

signals:
    void searchQueryChanged();
    void categoryFilterChanged();
    void scratchpadContentChanged();
    void activeProjectChanged();
    void noteCountChanged();

private:
    explicit NotesManager(QObject *parent = nullptr);
    void loadFromDatabase();
    void loadScratchpad();
    void saveScratchpadToDb();
    QString expandVariables(const QString &templateCode) const;

    NotesModel m_model;
    QString m_searchQuery;
    QString m_categoryFilter = "all";
    QString m_scratchpadContent;
    QString m_activeProject;
};

} // namespace Aura
