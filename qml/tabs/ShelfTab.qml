import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        // Header
        Row {
            width: parent.width
            height: 28

            Text {
                text: "Shelf"
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 15
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { width: 1; height: 1; Layout.fillWidth: true }

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                GlassButton {
                    text: "Clear Unpinned"
                    cornerRadius: 6
                    implicitHeight: 26
                    onClicked: {
                        if (shelfManager) shelfManager.clearUnpinned()
                    }
                }
            }
        }

        // Drop instruction banner / drop target
        Rectangle {
            width: parent.width
            height: 48
            radius: 10
            color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.08)
            border.color: Theme.glassBorder
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: 8

                IconImage {
                    source: "qrc:/resources/icons/drop.svg"
                    size: 16
                    color: Theme.accent
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: "Drop files or links here to pin them to your workspace"
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
            height: parent.height - 94
            clip: true
            spacing: 8
            model: shelfManager ? shelfManager.model : null

            delegate: GlassCard {
                width: listView.width
                implicitHeight: 46

                Row {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 10

                    IconImage {
                        source: {
                            if (model.itemType === "image") return "qrc:/resources/icons/media.svg"
                            if (model.itemType === "link") return "qrc:/resources/icons/mail.svg"
                            return "qrc:/resources/icons/shelf.svg"
                        }
                        size: 18
                        color: Theme.accent
                        anchors.verticalCenter: parent.verticalCenter
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

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        // Open button
                        GlassButton {
                            iconSource: "qrc:/resources/icons/play.svg"
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (shelfManager) shelfManager.openItem(model.itemId)
                        }

                        // Copy path button
                        GlassButton {
                            iconSource: "qrc:/resources/icons/copy.svg"
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (shelfManager) shelfManager.copyPath(model.itemId)
                        }

                        // Pin button
                        GlassButton {
                            iconSource: "qrc:/resources/icons/pin.svg"
                            iconColor: model.isPinned ? Theme.accent : Theme.textMuted
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (shelfManager) shelfManager.togglePin(model.itemId)
                        }

                        // Delete button
                        GlassButton {
                            iconSource: "qrc:/resources/icons/trash.svg"
                            isDanger: true
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (shelfManager) shelfManager.removeItem(model.itemId)
                        }
                    }
                }
            }

            Text {
                visible: listView.count === 0
                anchors.centerIn: parent
                text: "Shelf is empty. Drag files or code snippets here."
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: 12
            }
        }
    }
}
