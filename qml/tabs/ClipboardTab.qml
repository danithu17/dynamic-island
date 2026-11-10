import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    implicitWidth: 448
    implicitHeight: 230

    focus: true
    Keys.onPressed: function(event) {
        if (event.key >= Qt.Key_1 && event.key <= Qt.Key_9) {
            var idx = event.key - Qt.Key_1;
            if (clipboardManager) {
                clipboardManager.copyItemByIndex(idx);
                event.accepted = true;
            }
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        // Header: Title, Search, Paste Stack, On/Off Switch
        Item {
            width: parent.width
            height: 28

            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Text {
                    text: "Clipboard"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontTitleSize
                    font.weight: Theme.fontTitleWeight
                    anchors.verticalCenter: parent.verticalCenter
                }

                Rectangle {
                    width: clipCountTxt.implicitWidth + 12
                    height: 18
                    radius: 9
                    color: Qt.rgba(Theme.accentGreen.r, Theme.accentGreen.g, Theme.accentGreen.b, 0.20)
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        id: clipCountTxt
                        anchors.centerIn: parent
                        text: (clipboardManager ? clipboardManager.itemCount : 0).toString()
                        color: Theme.accentGreen
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
                    placeholderText: "Search clips..."
                    implicitWidth: 120
                    implicitHeight: 26
                    onTextChangedByUser: function(q) {
                        if (clipboardManager) clipboardManager.setSearchQuery(q)
                    }
                }

                // Clear text button
                Text {
                    text: "Clear"
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
                            if (clipboardManager) clipboardManager.clearHistory()
                        }
                    }
                }
            }
        }

        // List View: 52px rows, radius 18
        ListView {
            id: clipList
            width: parent.width
            height: parent.height - 36
            clip: true
            spacing: 6
            model: clipboardManager ? clipboardManager.model : null

            delegate: Rectangle {
                width: clipList.width
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

                        // Number index badge (16x16)
                        Rectangle {
                            width: 22
                            height: 22
                            radius: 11
                            color: Theme.cardPressed
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                anchors.centerIn: parent
                                text: String(index + 1)
                                color: Theme.accentGreen
                                font.family: Theme.fontMono
                                font.pixelSize: 10
                                font.weight: Font.DemiBold
                            }
                        }

                        // Text Preview and Type Tag
                        Column {
                            width: root.width - 230
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: model.preview
                                color: Theme.textPrimary
                                font.family: (model.itemType === "json" || model.itemType === "code") ? Theme.fontMono : Theme.fontFamily
                                font.pixelSize: Theme.fontBodySize // 13px
                                font.weight: Theme.fontBodyWeight
                                elide: Text.ElideRight
                                width: parent.width
                                maximumLineCount: 1
                            }

                            Row {
                                spacing: 6
                                Rectangle {
                                    width: typeLabel.implicitWidth + 8
                                    height: 14
                                    radius: 4
                                    color: Qt.rgba(Theme.accentGreen.r, Theme.accentGreen.g, Theme.accentGreen.b, 0.20)
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        id: typeLabel
                                        anchors.centerIn: parent
                                        text: model.itemType.toUpperCase()
                                        color: Theme.accentGreen
                                        font.family: Theme.fontMono
                                        font.pixelSize: 9
                                        font.weight: Font.DemiBold
                                    }
                                }

                                Text {
                                    text: model.charCount + " chars"
                                    color: Theme.textSecondary
                                    font.family: Theme.fontFamily
                                    font.pixelSize: Theme.fontCaptionSize
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }
                    }

                    // 30px circular action buttons
                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        // Copy
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
                                onClicked: if (clipboardManager) clipboardManager.copyItem(model.itemId)
                            }
                        }

                        // Format / Transform (JSON/Base64/Case)
                        Rectangle {
                            width: 30
                            height: 30
                            radius: 15
                            color: fmtMouse.pressed ? Theme.card : Theme.cardPressed
                            border.color: Theme.hairline
                            border.width: Theme.hairlineWidth
                            visible: model.itemType === "json" || model.itemType === "base64" || model.itemType === "text"

                            scale: fmtMouse.pressed ? 0.94 : 1.0
                            Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                            IconImage {
                                anchors.centerIn: parent
                                source: "qrc:/resources/icons/sparkles.svg"
                                size: 13
                                color: Theme.accentGreen
                            }
                            MouseArea {
                                id: fmtMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (clipboardManager) {
                                        if (model.itemType === "json") clipboardManager.formatJson(model.itemId)
                                        else if (model.itemType === "base64") clipboardManager.decodeBase64(model.itemId)
                                        else clipboardManager.changeCase(model.itemId, "upper")
                                    }
                                }
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
                                onClicked: if (clipboardManager) clipboardManager.togglePin(model.itemId)
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
                                color: Theme.accentRed // Red glyph
                            }
                            MouseArea {
                                id: delMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: if (clipboardManager) clipboardManager.removeItem(model.itemId)
                            }
                        }
                    }
                }
            }
        }
    }
}
