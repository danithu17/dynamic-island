import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    property string subTab: "notes" // "notes", "snippets", "scratchpad"
    property bool isEditing: false
    property int editingId: 0
    property bool isSnippetMode: false

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Top Sub-Navigation Bar & Action Controls
        Row {
            width: parent.width
            height: 28

            // Navigation Chips: Notes, Snippets, Scratchpad
            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                Rectangle {
                    width: notesChipTxt.implicitWidth + 14
                    height: 24
                    radius: 12
                    color: (root.subTab === "notes") ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                    border.color: (root.subTab === "notes") ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.12)
                    border.width: 1

                    Text {
                        id: notesChipTxt
                        anchors.centerIn: parent
                        text: "Markdown Notes"
                        color: (root.subTab === "notes") ? "#FFFFFF" : Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        font.bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.subTab = "notes"
                            root.isEditing = false
                            if (notesManager) notesManager.setCategoryFilter("notes")
                        }
                    }
                }

                Rectangle {
                    width: snipChipTxt.implicitWidth + 14
                    height: 24
                    radius: 12
                    color: (root.subTab === "snippets") ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                    border.color: (root.subTab === "snippets") ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.12)
                    border.width: 1

                    Text {
                        id: snipChipTxt
                        anchors.centerIn: parent
                        text: "Snippets"
                        color: (root.subTab === "snippets") ? "#FFFFFF" : Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        font.bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.subTab = "snippets"
                            root.isEditing = false
                            if (notesManager) notesManager.setCategoryFilter("snippets")
                        }
                    }
                }

                Rectangle {
                    width: scratchChipTxt.implicitWidth + 14
                    height: 24
                    radius: 12
                    color: (root.subTab === "scratchpad") ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                    border.color: (root.subTab === "scratchpad") ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.12)
                    border.width: 1

                    Text {
                        id: scratchChipTxt
                        anchors.centerIn: parent
                        text: "Scratchpad"
                        color: (root.subTab === "scratchpad") ? "#FFFFFF" : Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        font.bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.subTab = "scratchpad"
                            root.isEditing = false
                        }
                    }
                }
            }

            // Right-aligned actions (Search, New, Export)
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                GlassInput {
                    visible: root.subTab !== "scratchpad" && !root.isEditing
                    placeholderText: "Search notes..."
                    implicitWidth: 120
                    implicitHeight: 26
                    onTextChangedByUser: function(q) {
                        if (notesManager) notesManager.setSearchQuery(q)
                    }
                }

                GlassButton {
                    visible: root.subTab === "scratchpad"
                    text: "Clear Scratchpad"
                    cornerRadius: 6
                    implicitHeight: 26
                    onClicked: if (notesManager) notesManager.clearScratchpad()
                }

                GlassButton {
                    visible: root.subTab !== "scratchpad"
                    text: root.isEditing ? "Back" : (root.subTab === "snippets" ? "+ Snippet" : "+ Note")
                    isAccent: !root.isEditing
                    cornerRadius: 6
                    implicitHeight: 26
                    onClicked: {
                        if (root.isEditing) {
                            root.isEditing = false
                        } else {
                            root.editingId = 0
                            root.isSnippetMode = (root.subTab === "snippets")
                            noteTitleInput.text = ""
                            noteLangInput.text = root.isSnippetMode ? "javascript" : "markdown"
                            noteCodeInput.text = root.isSnippetMode ? "// Use {date}, {time}, {uuid}, {clipboard}\n" : ""
                            noteTagsInput.text = ""
                            root.isEditing = true
                        }
                    }
                }
            }
        }

        // =====================================================
        // VIEW 1: Scratchpad Persistent Notepad
        // =====================================================
        Column {
            visible: root.subTab === "scratchpad"
            width: parent.width
            height: parent.height - 36
            spacing: 6

            Text {
                text: "Temporary scratchpad (auto-saves instantly, 100% offline)"
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: 10
            }

            Rectangle {
                width: parent.width
                height: parent.height - 20
                radius: 12
                color: Qt.rgba(0.06, 0.07, 0.10, 0.6)
                border.color: Theme.glassBorder
                border.width: 1

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 10
                    clip: true

                    TextArea {
                        id: scratchText
                        text: notesManager ? notesManager.scratchpadContent : ""
                        color: Theme.textPrimary
                        font.family: Theme.fontMono
                        font.pixelSize: 11
                        wrapMode: TextEdit.Wrap
                        selectByMouse: true
                        background: null
                        onTextChanged: {
                            if (notesManager && text !== notesManager.scratchpadContent) {
                                notesManager.scratchpadContent = text
                            }
                        }
                    }
                }
            }
        }

        // =====================================================
        // VIEW 2: Note / Snippet Editor
        // =====================================================
        Column {
            visible: root.isEditing && root.subTab !== "scratchpad"
            width: parent.width
            height: parent.height - 36
            spacing: 6

            Row {
                width: parent.width
                spacing: 8

                GlassInput {
                    id: noteTitleInput
                    placeholderText: root.isSnippetMode ? "Snippet Title..." : "Note Title..."
                    implicitWidth: parent.width - 120
                    implicitHeight: 28
                }

                GlassInput {
                    id: noteLangInput
                    placeholderText: "Language"
                    text: root.isSnippetMode ? "javascript" : "markdown"
                    implicitWidth: 112
                    implicitHeight: 28
                }
            }

            GlassInput {
                id: noteTagsInput
                placeholderText: "Tags (comma separated)..."
                width: parent.width
                implicitHeight: 26
            }

            Rectangle {
                width: parent.width
                height: parent.height - 98
                radius: 10
                color: Qt.rgba(0.06, 0.07, 0.10, 0.6)
                border.color: Theme.glassBorder
                border.width: 1

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 8
                    clip: true

                    TextArea {
                        id: noteCodeInput
                        color: Theme.textPrimary
                        font.family: Theme.fontMono
                        font.pixelSize: 11
                        wrapMode: TextEdit.Wrap
                        selectByMouse: true
                        background: null
                    }
                }
            }

            Row {
                anchors.right: parent.right
                spacing: 8

                Text {
                    visible: root.isSnippetMode
                    text: "Variables: {date}, {time}, {uuid}, {clipboard}"
                    color: Theme.textMuted
                    font.family: Theme.fontMono
                    font.pixelSize: 9
                    anchors.verticalCenter: parent.verticalCenter
                }

                GlassButton {
                    text: "Cancel"
                    cornerRadius: 6
                    implicitHeight: 26
                    onClicked: root.isEditing = false
                }

                GlassButton {
                    text: "Save"
                    isAccent: true
                    cornerRadius: 6
                    implicitHeight: 26
                    onClicked: {
                        if (notesManager) {
                            notesManager.saveNote(root.editingId,
                                                  noteTitleInput.text,
                                                  noteLangInput.text,
                                                  noteCodeInput.text,
                                                  noteTagsInput.text,
                                                  notesManager.activeProject,
                                                  root.isSnippetMode)
                        }
                        root.isEditing = false
                    }
                }
            }
        }

        // =====================================================
        // VIEW 3: Notes & Snippets List
        // =====================================================
        ListView {
            id: notesList
            visible: !root.isEditing && root.subTab !== "scratchpad"
            width: parent.width
            height: parent.height - 36
            clip: true
            spacing: 6
            model: notesManager ? notesManager.model : null

            delegate: GlassCard {
                width: notesList.width
                implicitHeight: cardCol.implicitHeight + 12

                Column {
                    id: cardCol
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 8
                    spacing: 4

                    Row {
                        width: parent.width
                        spacing: 8

                        // Language / Type Tag
                        Rectangle {
                            width: langTxt.implicitWidth + 8
                            height: 16
                            radius: 4
                            color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.2)
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                id: langTxt
                                anchors.centerIn: parent
                                text: model.language ? model.language.toUpperCase() : (model.isSnippet ? "SNIPPET" : "MD")
                                color: Theme.accent
                                font.family: Theme.fontMono
                                font.pixelSize: 8
                                font.bold: true
                            }
                        }

                        // Title
                        Text {
                            width: parent.width - 150
                            anchors.verticalCenter: parent.verticalCenter
                            text: model.title
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        // Right actions (Copy, Edit, Pin, Delete)
                        Row {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 4

                            GlassButton {
                                iconSource: "qrc:/resources/icons/copy.svg"
                                cornerRadius: 4
                                implicitWidth: 22
                                implicitHeight: 22
                                onClicked: {
                                    if (model.isSnippet) {
                                        if (notesManager) notesManager.copySnippetWithVariables(model.noteId)
                                    } else {
                                        if (notesManager) notesManager.copyCode(model.noteId)
                                    }
                                }
                            }

                            GlassButton {
                                iconSource: "qrc:/resources/icons/notes.svg"
                                cornerRadius: 4
                                implicitWidth: 22
                                implicitHeight: 22
                                onClicked: {
                                    root.editingId = model.noteId
                                    root.isSnippetMode = model.isSnippet
                                    noteTitleInput.text = model.title
                                    noteLangInput.text = model.language
                                    noteCodeInput.text = model.code
                                    noteTagsInput.text = model.tags
                                    root.isEditing = true
                                }
                            }

                            GlassButton {
                                iconSource: "qrc:/resources/icons/pin.svg"
                                iconColor: model.isPinned ? Theme.accent : Theme.textMuted
                                cornerRadius: 4
                                implicitWidth: 22
                                implicitHeight: 22
                                onClicked: if (notesManager) notesManager.togglePin(model.noteId)
                            }

                            GlassButton {
                                iconSource: "qrc:/resources/icons/trash.svg"
                                isDanger: true
                                cornerRadius: 4
                                implicitWidth: 22
                                implicitHeight: 22
                                onClicked: if (notesManager) notesManager.deleteNote(model.noteId)
                            }
                        }
                    }

                    // Content Snippet Preview / Markdown Snippet
                    Text {
                        width: parent.width
                        text: model.code.replace(/\n/g, ' ')
                        color: Theme.textSecondary
                        font.family: Theme.fontMono
                        font.pixelSize: 10
                        elide: Text.ElideRight
                        maximumLineCount: 2
                    }

                    // Checkbox TODO stats row (if note has todo items)
                    Row {
                        visible: model.hasTodos
                        spacing: 6

                        Rectangle {
                            width: 10; height: 10; radius: 2
                            color: (model.doneTodos === model.totalTodos) ? Theme.success : Theme.accent
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "TODO: " + model.doneTodos + "/" + model.totalTodos + " completed"
                            color: Theme.textMuted
                            font.family: Theme.fontMono
                            font.pixelSize: 9
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            Text {
                visible: notesList.count === 0
                anchors.centerIn: parent
                text: "No notes or snippets found. Create one with '+ Note' or press Ctrl+Alt+N for Quick Capture."
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: 11
            }
        }
    }
}
