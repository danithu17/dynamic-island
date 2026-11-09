import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    // Support keyboard shortcuts 1-9 to select items
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
        anchors.margins: 12
        spacing: 8

        // Header: Title & Actions
        Row {
            width: parent.width
            height: 28

            Text {
                text: "Programmer Clipboard"
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 14
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                GlassInput {
                    id: searchInput
                    placeholderText: "Fuzzy search..."
                    implicitWidth: 140
                    implicitHeight: 26
                    onTextChangedByUser: function(q) {
                        if (clipboardManager) clipboardManager.setSearchQuery(q)
                    }
                }

                GlassButton {
                    text: "+ UUID"
                    cornerRadius: 6
                    implicitHeight: 26
                    implicitWidth: 54
                    onClicked: if (clipboardManager) clipboardManager.generateUuid()
                }

                GlassButton {
                    text: "Clear"
                    cornerRadius: 6
                    implicitHeight: 26
                    implicitWidth: 46
                    onClicked: if (clipboardManager) clipboardManager.clearHistory()
                }
            }
        }

        // Filter Chips Row
        Row {
            width: parent.width
            height: 24
            spacing: 6

            readonly property var chips: [
                { id: "all", label: "All" },
                { id: "code", label: "Code & JSON" },
                { id: "links", label: "Links" },
                { id: "pinned", label: "Pinned" }
            ]

            Repeater {
                model: parent.chips

                Rectangle {
                    width: chipTxt.implicitWidth + 14
                    height: 22
                    radius: 11
                    color: (clipboardManager && clipboardManager.categoryFilter === modelData.id)
                           ? Theme.accent
                           : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                    border.color: (clipboardManager && clipboardManager.categoryFilter === modelData.id)
                                  ? Theme.accent
                                  : Qt.rgba(1.0, 1.0, 1.0, 0.12)
                    border.width: 1

                    Text {
                        id: chipTxt
                        anchors.centerIn: parent
                        text: modelData.label
                        color: (clipboardManager && clipboardManager.categoryFilter === modelData.id)
                               ? "#FFFFFF"
                               : Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        font.weight: Font.Medium
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (clipboardManager) clipboardManager.setCategoryFilter(modelData.id)
                        }
                    }
                }
            }

            // Paste Stack status pill
            Rectangle {
                visible: settings && settings.pasteStackEnabled
                width: stackTxt.implicitWidth + 14
                height: 22
                radius: 11
                color: Qt.rgba(Theme.warning.r, Theme.warning.g, Theme.warning.b, 0.25)
                border.color: Theme.warning
                border.width: 1

                Text {
                    id: stackTxt
                    anchors.centerIn: parent
                    text: "Stack: " + (clipboardManager ? clipboardManager.pasteStackCount : 0) + " (Ctrl+Alt+V)"
                    color: Theme.warning
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.bold: true
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: if (clipboardManager) clipboardManager.popPasteStack()
                }
            }
        }

        // Clipboard List
        ListView {
            id: clipList
            width: parent.width
            height: parent.height - 66
            clip: true
            spacing: 6
            model: clipboardManager ? clipboardManager.model : null

            delegate: GlassCard {
                id: card
                width: clipList.width
                implicitHeight: cardContent.implicitHeight + 12

                Column {
                    id: cardContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 6
                    spacing: 4

                    // Main Row: Index Badge, Type Icon/Swatch, Text Preview, Pin & Copy
                    Row {
                        width: parent.width
                        spacing: 8

                        // Quick Select Number (1-9)
                        Rectangle {
                            visible: index < 9
                            width: 16; height: 16; radius: 4
                            color: Qt.rgba(1.0, 1.0, 1.0, 0.12)
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                anchors.centerIn: parent
                                text: String(index + 1)
                                color: Theme.accent
                                font.family: Theme.fontMono
                                font.pixelSize: 9
                                font.bold: true
                            }
                        }

                        // Hex Color Swatch or Type Badge
                        Rectangle {
                            visible: model.itemType === "hex_color"
                            width: 16; height: 16; radius: 3
                            color: (model.itemType === "hex_color") ? model.textData.trim() : "transparent"
                            border.color: Qt.rgba(1.0, 1.0, 1.0, 0.3)
                            border.width: 1
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        // Type Tag
                        Rectangle {
                            visible: model.itemType !== "hex_color"
                            width: typeLabel.implicitWidth + 8; height: 16; radius: 4
                            color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.18)
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                id: typeLabel
                                anchors.centerIn: parent
                                text: model.itemType.toUpperCase()
                                color: Theme.accent
                                font.family: Theme.fontMono
                                font.pixelSize: 8
                                font.bold: true
                            }
                        }

                        // Preview Text
                        Text {
                            width: parent.width - (index < 9 ? 160 : 140)
                            anchors.verticalCenter: parent.verticalCenter
                            text: model.preview
                            color: Theme.textPrimary
                            font.family: (model.itemType === "code" || model.itemType === "json" || model.itemType === "shell_command")
                                         ? Theme.fontMono : Theme.fontFamily
                            font.pixelSize: 11
                            font.weight: Font.Medium
                            elide: Text.ElideRight
                        }

                        // Action Buttons: Copy, Pin, Delete
                        Row {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 4

                            GlassButton {
                                iconSource: "qrc:/resources/icons/copy.svg"
                                cornerRadius: 4
                                implicitWidth: 22
                                implicitHeight: 22
                                onClicked: if (clipboardManager) clipboardManager.copyItem(model.itemId)
                            }

                            GlassButton {
                                iconSource: "qrc:/resources/icons/pin.svg"
                                iconColor: model.isPinned ? Theme.accent : Theme.textMuted
                                cornerRadius: 4
                                implicitWidth: 22
                                implicitHeight: 22
                                onClicked: if (clipboardManager) clipboardManager.togglePin(model.itemId)
                            }

                            GlassButton {
                                iconSource: "qrc:/resources/icons/trash.svg"
                                isDanger: true
                                cornerRadius: 4
                                implicitWidth: 22
                                implicitHeight: 22
                                onClicked: if (clipboardManager) clipboardManager.removeItem(model.itemId)
                            }
                        }
                    }

                    // Quick Actions Row per Type
                    Row {
                        width: parent.width
                        spacing: 4

                        // Stats
                        Text {
                            text: model.lineCount + (model.lineCount === 1 ? " line, " : " lines, ") + model.charCount + " chars"
                            color: Theme.textMuted
                            font.family: Theme.fontMono
                            font.pixelSize: 8
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Item { width: 4; height: 1 }

                        // JSON Actions
                        GlassButton {
                            visible: model.itemType === "json"
                            text: "Format JSON"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.formatJson(model.itemId)
                        }

                        GlassButton {
                            visible: model.itemType === "json"
                            text: "Minify"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.minifyJson(model.itemId)
                        }

                        // URL Actions
                        GlassButton {
                            visible: model.itemType === "url"
                            text: "Open Link"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: Qt.openUrlExternally(model.textData.trim())
                        }

                        GlassButton {
                            visible: model.itemType === "url"
                            text: "URL Enc"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.urlEncode(model.itemId)
                        }

                        // Base64 Actions
                        GlassButton {
                            text: "Base64"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.base64Encode(model.itemId)
                        }

                        // Case Transformations
                        GlassButton {
                            text: "camel"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.changeCase(model.itemId, "camel")
                        }

                        GlassButton {
                            text: "snake"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.changeCase(model.itemId, "snake")
                        }

                        GlassButton {
                            text: "kebab"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.changeCase(model.itemId, "kebab")
                        }

                        GlassButton {
                            text: "Pascal"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.changeCase(model.itemId, "pascal")
                        }

                        GlassButton {
                            text: "UPPER"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.changeCase(model.itemId, "upper")
                        }

                        // Trim Whitespace
                        GlassButton {
                            text: "Trim"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.trimWhitespace(model.itemId)
                        }

                        // Wrap Markdown
                        GlassButton {
                            text: "``` code"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.wrapMarkdown(model.itemId, model.itemSubType)
                        }

                        // Timestamp Convert
                        GlassButton {
                            visible: !isNaN(model.textData.trim()) && model.textData.trim().length >= 10
                            text: "Epoch Date"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.unixTimestampConvert(model.itemId)
                        }

                        // Send to Notes / Snippets
                        GlassButton {
                            text: "To Notes"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.sendToNotes(model.itemId)
                        }

                        GlassButton {
                            text: "To Snippet"
                            cornerRadius: 4
                            implicitHeight: 18
                            onClicked: if (clipboardManager) clipboardManager.saveAsSnippet(model.itemId)
                        }
                    }
                }

                // Click to copy back directly
                MouseArea {
                    anchors.fill: parent
                    z: -1
                    cursorShape: Qt.PointingHandCursor
                    onClicked: if (clipboardManager) clipboardManager.copyItem(model.itemId)
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
