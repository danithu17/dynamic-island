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
                text: "Clipboard History"
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

                GlassInput {
                    placeholderText: "Search clips..."
                    implicitWidth: 160
                    implicitHeight: 26
                    onTextChangedByUser: function(q) {
                        if (clipboardManager) clipboardManager.setSearchQuery(q)
                    }
                }

                GlassButton {
                    text: "Clear"
                    cornerRadius: 6
                    implicitHeight: 26
                    onClicked: if (clipboardManager) clipboardManager.clearHistory()
                }
            }
        }

        // Clipboard List
        ListView {
            id: clipList
            width: parent.width
            height: parent.height - 40
            clip: true
            spacing: 8
            model: clipboardManager ? clipboardManager.model : null

            delegate: GlassCard {
                width: clipList.width
                implicitHeight: 44

                Row {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 10

                    IconImage {
                        source: {
                            if (model.itemType === "code") return "qrc:/resources/icons/dev.svg"
                            if (model.itemType === "link") return "qrc:/resources/icons/mail.svg"
                            return "qrc:/resources/icons/clipboard.svg"
                        }
                        size: 16
                        color: model.isPinned ? Theme.accent : Theme.textMuted
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        width: parent.width - 120
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: model.preview
                            color: Theme.textPrimary
                            font.family: model.itemType === "code" ? Theme.fontMono : Theme.fontFamily
                            font.pixelSize: 11
                            font.weight: Font.Medium
                            elide: Text.ElideRight
                            width: parent.width
                        }

                        Text {
                            text: model.createdAt
                            color: Theme.textMuted
                            font.family: Theme.fontFamily
                            font.pixelSize: 9
                        }
                    }

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        // Copy back
                        GlassButton {
                            iconSource: "qrc:/resources/icons/copy.svg"
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (clipboardManager) clipboardManager.copyItem(model.itemId)
                        }

                        // Pin
                        GlassButton {
                            iconSource: "qrc:/resources/icons/pin.svg"
                            iconColor: model.isPinned ? Theme.accent : Theme.textMuted
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (clipboardManager) clipboardManager.togglePin(model.itemId)
                        }

                        // Remove
                        GlassButton {
                            iconSource: "qrc:/resources/icons/trash.svg"
                            isDanger: true
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (clipboardManager) clipboardManager.removeItem(model.itemId)
                        }
                    }
                }
            }

            Text {
                visible: clipList.count === 0
                anchors.centerIn: parent
                text: "No clipboard history. Copy something to see it here."
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: 12
            }
        }
    }
}
