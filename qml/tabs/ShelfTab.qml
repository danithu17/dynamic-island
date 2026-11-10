import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Header
        Row {
            width: parent.width
            height: 28

            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Text {
                    text: "Tray"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 15
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }

                Rectangle {
                    width: trayCountTxt.implicitWidth + 10
                    height: 18
                    radius: 9
                    color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.2)
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        id: trayCountTxt
                        anchors.centerIn: parent
                        text: (shelfManager && shelfManager.model) ? shelfManager.model.rowCount() : "0"
                        color: Theme.accent
                        font.family: Theme.fontMono
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                GlassInput {
                    id: searchInput
                    placeholderText: "Search tray..."
                    implicitWidth: 125
                    implicitHeight: 26
                    onTextChangedByUser: function(q) {
                        if (shelfManager) shelfManager.setSearchQuery(q)
                    }
                }

                GlassButton {
                    text: "Folder"
                    cornerRadius: 6
                    implicitHeight: 26
                    implicitWidth: 50
                    onClicked: {
                        if (shelfManager) shelfManager.openTrayFolder()
                    }
                }

                GlassButton {
                    text: "Clear"
                    cornerRadius: 6
                    implicitHeight: 26
                    implicitWidth: 44
                    onClicked: {
                        if (shelfManager) shelfManager.clearUnpinned()
                    }
                }
            }
        }

        // Drop target banner
        Rectangle {
            width: parent.width
            height: 38
            radius: 12
            color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.08)
            border.color: Theme.glassBorder
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: 6

                IconImage {
                    source: "qrc:/resources/icons/drop.svg"
                    size: 14
                    color: Theme.accent
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: "Drop files or links here to save to %APPDATA%/tray"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            DropArea {
                anchors.fill: parent
                onDropped: function(drop) {
                    if (drop.hasUrls && shelfManager) shelfManager.addDropUrls(drop.urls)
                    else if (drop.hasText && shelfManager) shelfManager.addText(drop.text)
                    drop.acceptProposedAction()
                }
            }
        }

        // Item List
        ListView {
            id: listView
            width: parent.width
            height: parent.height - 82
            clip: true
            spacing: 6
            model: shelfManager ? shelfManager.model : null

            delegate: GlassCard {
                width: listView.width
                implicitHeight: 44

                Item {
                    anchors.fill: parent
                    anchors.margins: 6

                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        IconImage {
                            source: {
                                if (model.itemType === "image") return "qrc:/resources/icons/media.svg"
                                if (model.itemType === "link") return "qrc:/resources/icons/mail.svg"
                                return "qrc:/resources/icons/shelf.svg"
                            }
                            size: 16
                            color: Theme.accent
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Column {
                            width: root.width - 180
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: model.title
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                                font.weight: Font.Medium
                                elide: Text.ElideMiddle
                                width: parent.width
                            }

                            Text {
                                text: model.filePath !== "" ? model.filePath : model.itemData
                                color: Theme.textMuted
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                elide: Text.ElideMiddle
                                width: parent.width
                            }
                        }
                    }

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4

                        // Open button
                        GlassButton {
                            iconSource: "qrc:/resources/icons/play.svg"
                            cornerRadius: 4
                            implicitWidth: 22
                            implicitHeight: 22
                            onClicked: if (shelfManager) shelfManager.openItem(model.itemId)
                        }

                        // Reveal in explorer
                        GlassButton {
                            iconSource: "qrc:/resources/icons/search.svg"
                            cornerRadius: 4
                            implicitWidth: 22
                            implicitHeight: 22
                            visible: model.filePath !== ""
                            onClicked: if (shelfManager) shelfManager.revealInExplorer(model.itemId)
                        }

                        // Copy path button
                        GlassButton {
                            iconSource: "qrc:/resources/icons/copy.svg"
                            cornerRadius: 4
                            implicitWidth: 22
                            implicitHeight: 22
                            onClicked: if (shelfManager) shelfManager.copyPath(model.itemId)
                        }

                        // Pin button
                        GlassButton {
                            iconSource: "qrc:/resources/icons/pin.svg"
                            iconColor: model.isPinned ? Theme.accent : Theme.textMuted
                            cornerRadius: 4
                            implicitWidth: 22
                            implicitHeight: 22
                            onClicked: if (shelfManager) shelfManager.togglePin(model.itemId)
                        }

                        // Delete button
                        GlassButton {
                            iconSource: "qrc:/resources/icons/trash.svg"
                            isDanger: true
                            cornerRadius: 4
                            implicitWidth: 22
                            implicitHeight: 22
                            onClicked: if (shelfManager) shelfManager.removeItem(model.itemId)
                        }
                    }
                }
            }

            Text {
                visible: listView.count === 0
                anchors.centerIn: parent
                text: "Tray is empty. Save files or snippets here."
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: 11
            }
        }
    }
}
