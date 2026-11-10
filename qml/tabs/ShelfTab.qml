import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    implicitWidth: 448
    implicitHeight: 230

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        // Header: Title, Count chip, Search, Clear unpinned text button
        Item {
            width: parent.width
            height: 28

            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Text {
                    text: "Tray"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontTitleSize
                    font.weight: Theme.fontTitleWeight
                    anchors.verticalCenter: parent.verticalCenter
                }

                Rectangle {
                    width: trayCountTxt.implicitWidth + 12
                    height: 18
                    radius: 9
                    color: Qt.rgba(Theme.accentBlue.r, Theme.accentBlue.g, Theme.accentBlue.b, 0.20)
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        id: trayCountTxt
                        anchors.centerIn: parent
                        text: (shelfManager && shelfManager.model) ? shelfManager.model.rowCount() : "0"
                        color: Theme.accentBlue
                        font.family: Theme.fontMono
                        font.pixelSize: Theme.fontCaptionSize
                        font.weight: Font.DemiBold
                    }
                }
            }

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                GlassInput {
                    id: searchInput
                    placeholderText: "Search..."
                    implicitWidth: 120
                    implicitHeight: 26
                    onTextChangedByUser: function(q) {
                        if (shelfManager) shelfManager.setSearchQuery(q)
                    }
                }

                // "Clear unpinned" as a text button
                Text {
                    text: "Clear unpinned"
                    color: clearMouse.containsMouse ? Theme.textPrimary : Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontCaptionSize
                    anchors.verticalCenter: parent.verticalCenter

                    MouseArea {
                        id: clearMouse
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: {
                            if (shelfManager) shelfManager.clearUnpinned()
                        }
                    }
                }
            }
        }

        // Item List: dark rows (52px, radius 18)
        ListView {
            id: listView
            width: parent.width
            height: parent.height - 36
            clip: true
            spacing: 6
            model: shelfManager ? shelfManager.model : null

            delegate: Rectangle {
                width: listView.width
                height: 52 // 52px as specified
                radius: 18 // radius 18 as specified
                color: Theme.card
                border.color: Theme.hairline
                border.width: Theme.hairlineWidth
                antialiasing: true

                Item {
                    anchors.fill: parent
                    anchors.margins: 8

                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 10

                        // Thumbnail / Icon (36x36 rounded)
                        Rectangle {
                            width: 36
                            height: 36
                            radius: 10
                            color: Theme.cardPressed
                            anchors.verticalCenter: parent.verticalCenter

                            IconImage {
                                anchors.centerIn: parent
                                source: {
                                    if (model.itemType === "image") return "qrc:/resources/icons/media.svg"
                                    if (model.itemType === "link") return "qrc:/resources/icons/mail.svg"
                                    return "qrc:/resources/icons/shelf.svg"
                                }
                                size: 18
                                color: Theme.accentBlue
                            }
                        }

                        // Name and size
                        Column {
                            width: root.width - 200
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: model.title
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontBodySize // 13px
                                font.weight: Theme.fontBodyWeight
                                elide: Text.ElideMiddle
                                width: parent.width
                            }

                            Text {
                                text: model.filePath !== "" ? model.filePath : model.itemData
                                color: Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontCaptionSize
                                font.weight: Theme.fontCaptionWeight
                                elide: Text.ElideMiddle
                                width: parent.width
                            }
                        }
                    }

                    // Actions: 30px circular icon buttons (#2C2C2E background, white glyph; delete is red glyph)
                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        // Open
                        Rectangle {
                            width: 30
                            height: 30
                            radius: 15
                            color: openMouse.pressed ? Theme.card : Theme.cardPressed
                            border.color: Theme.hairline
                            border.width: Theme.hairlineWidth

                            scale: openMouse.pressed ? 0.94 : 1.0
                            Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                            IconImage {
                                anchors.centerIn: parent
                                source: "qrc:/resources/icons/play.svg"
                                size: 13
                                color: "#FFFFFF"
                            }
                            MouseArea {
                                id: openMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: if (shelfManager) shelfManager.openItem(model.itemId)
                            }
                        }

                        // Copy Path
                        Rectangle {
                            width: 30
                            height: 30
                            radius: 15
                            color: copyMouse.pressed ? Theme.card : Theme.cardPressed
                            border.color: Theme.hairline
                            border.width: Theme.hairlineWidth

                            scale: copyMouse.pressed ? 0.94 : 1.0
                            Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                            IconImage {
                                anchors.centerIn: parent
                                source: "qrc:/resources/icons/copy.svg"
                                size: 13
                                color: "#FFFFFF"
                            }
                            MouseArea {
                                id: copyMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: if (shelfManager) shelfManager.copyPath(model.itemId)
                            }
                        }

                        // Pin
                        Rectangle {
                            width: 30
                            height: 30
                            radius: 15
                            color: model.isPinned ? Qt.rgba(Theme.accentOrange.r, Theme.accentOrange.g, Theme.accentOrange.b, 0.25) : (pinMouse.pressed ? Theme.card : Theme.cardPressed)
                            border.color: model.isPinned ? Theme.accentOrange : Theme.hairline
                            border.width: Theme.hairlineWidth

                            scale: pinMouse.pressed ? 0.94 : 1.0
                            Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                            IconImage {
                                anchors.centerIn: parent
                                source: "qrc:/resources/icons/pin.svg"
                                size: 13
                                color: model.isPinned ? Theme.accentOrange : "#FFFFFF"
                            }
                            MouseArea {
                                id: pinMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: if (shelfManager) shelfManager.togglePin(model.itemId)
                            }
                        }

                        // Delete (Red glyph)
                        Rectangle {
                            width: 30
                            height: 30
                            radius: 15
                            color: delMouse.pressed ? Theme.card : Theme.cardPressed
                            border.color: Theme.hairline
                            border.width: Theme.hairlineWidth

                            scale: delMouse.pressed ? 0.94 : 1.0
                            Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                            IconImage {
                                anchors.centerIn: parent
                                source: "qrc:/resources/icons/trash.svg"
                                size: 13
                                color: Theme.accentRed // Red glyph as specified
                            }
                            MouseArea {
                                id: delMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: if (shelfManager) shelfManager.removeItem(model.itemId)
                            }
                        }
                    }
                }
            }
        }
    }
}
