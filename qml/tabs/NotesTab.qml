import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root
    property bool isEditing: false
    property int editingId: 0

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        // Header
        Row {
            width: parent.width
            height: 28

            Text {
                text: "Notes & Snippets"
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 15
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }


            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                GlassInput {
                    visible: !root.isEditing
                    placeholderText: "Search notes..."
                    implicitWidth: 150
                    implicitHeight: 26
                    onTextChangedByUser: function(q) {
                        if (notesManager) notesManager.setSearchQuery(q)
                    }
                }

                GlassButton {
                    text: root.isEditing ? "Back to Notes" : "+ New Note"
                    isAccent: !root.isEditing
                    cornerRadius: 6
                    implicitHeight: 26
                    onClicked: {
                        if (root.isEditing) {
                            root.isEditing = false
                        } else {
                            root.editingId = 0
                            noteTitleInput.text = ""
                            noteLangInput.text = "cpp"
                            noteCodeInput.text = ""
                            noteTagsInput.text = ""
                            root.isEditing = true
                        }
                    }
                }
            }
        }

        // Editor view when adding/editing
        Column {
            visible: root.isEditing
            width: parent.width
            height: parent.height - 40
            spacing: 8

            Row {
                width: parent.width
                spacing: 8

                Rectangle {
                    width: parent.width - 120
                    height: 30
                    radius: 6
                    color: Theme.glassSurface
                    border.color: Theme.glassBorder

                    TextInput {
                        id: noteTitleInput
                        anchors.fill: parent
                        anchors.margins: 6
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        Text {
                            text: "Note / Snippet Title"
                            color: Theme.textMuted
                            visible: !noteTitleInput.text
                        }
                    }
                }

                Rectangle {
                    width: 110
                    height: 30
                    radius: 6
                    color: Theme.glassSurface
                    border.color: Theme.glassBorder

                    TextInput {
                        id: noteLangInput
                        anchors.fill: parent
                        anchors.margins: 6
                        color: Theme.accent
                        font.family: Theme.fontMono
                        font.pixelSize: 11
                        text: "cpp"
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: parent.height - 76
                radius: 8
                color: Qt.rgba(0.04, 0.05, 0.07, 0.8)
                border.color: Theme.glassBorder
                clip: true

                Flickable {
                    anchors.fill: parent
                    anchors.margins: 8
                    contentWidth: noteCodeInput.paintedWidth
                    contentHeight: noteCodeInput.paintedHeight
                    clip: true

                    TextEdit {
                        id: noteCodeInput
                        width: parent.width
                        color: "#E2E8F0"
                        font.family: Theme.fontMono
                        font.pixelSize: 12
                        wrapMode: TextEdit.Wrap
                        selectByMouse: true
                    }
                }
            }

            Row {
                width: parent.width
                spacing: 8

                Rectangle {
                    width: parent.width - 90
                    height: 28
                    radius: 6
                    color: Theme.glassSurface
                    border.color: Theme.glassBorder

                    TextInput {
                        id: noteTagsInput
                        anchors.fill: parent
                        anchors.margins: 6
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        Text {
                            text: "Tags (comma separated, e.g. dev, api, quickfix)"
                            color: Theme.textMuted
                            visible: !noteTagsInput.text
                        }
                    }
                }

                GlassButton {
                    text: "Save"
                    isAccent: true
                    cornerRadius: 6
                    implicitHeight: 28
                    implicitWidth: 80
                    onClicked: {
                        if (notesManager) {
                            notesManager.saveNote(root.editingId, noteTitleInput.text, noteLangInput.text, noteCodeInput.text, noteTagsInput.text)
                        }
                        root.isEditing = false
                    }
                }
            }
        }

        // Notes List view
        ListView {
            id: notesList
            visible: !root.isEditing
            width: parent.width
            height: parent.height - 40
            clip: true
            spacing: 8
            model: notesManager ? notesManager.model : null

            delegate: GlassCard {
                width: notesList.width
                implicitHeight: 52

                Row {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 10

                    Rectangle {
                        width: 38
                        height: 24
                        radius: 4
                        color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.2)
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            anchors.centerIn: parent
                            text: model.language.toUpperCase()
                            color: Theme.accent
                            font.family: Theme.fontMono
                            font.pixelSize: 9
                            font.bold: true
                        }
                    }

                    Column {
                        width: parent.width - 150
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: model.title
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            font.bold: true
                            elide: Text.ElideRight
                            width: parent.width
                        }

                        Text {
                            text: model.code.replace(/\n/g, ' ')
                            color: Theme.textMuted
                            font.family: Theme.fontMono
                            font.pixelSize: 10
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        // Copy code
                        GlassButton {
                            iconSource: "qrc:/resources/icons/copy.svg"
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (notesManager) notesManager.copyCode(model.noteId)
                        }

                        // Edit
                        GlassButton {
                            iconSource: "qrc:/resources/icons/notes.svg"
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: {
                                root.editingId = model.noteId
                                noteTitleInput.text = model.title
                                noteLangInput.text = model.language
                                noteCodeInput.text = model.code
                                noteTagsInput.text = model.tags
                                root.isEditing = true
                            }
                        }

                        // Pin
                        GlassButton {
                            iconSource: "qrc:/resources/icons/pin.svg"
                            iconColor: model.isPinned ? Theme.accent : Theme.textMuted
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (notesManager) notesManager.togglePin(model.noteId)
                        }

                        // Delete
                        GlassButton {
                            iconSource: "qrc:/resources/icons/trash.svg"
                            isDanger: true
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (notesManager) notesManager.deleteNote(model.noteId)
                        }
                    }
                }
            }

            Text {
                visible: notesList.count === 0
                anchors.centerIn: parent
                text: "No notes or snippets yet. Click '+ New Note' to create one."
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: 12
            }
        }
    }
}
