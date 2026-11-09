import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    readonly property var accentColors: [
        { name: "iOS Blue", color: "#0A84FF" },
        { name: "Purple", color: "#BF5AF2" },
        { name: "Orange", color: "#FF9F0A" },
        { name: "Green", color: "#30D158" },
        { name: "Rose", color: "#FF375F" },
        { name: "Cyan", color: "#64D2FF" }
    ]

    Flickable {
        anchors.fill: parent
        anchors.margins: 14
        contentHeight: settingsCol.height + 20
        clip: true

        Column {
            id: settingsCol
            width: parent.width
            spacing: 12

            // Header
            Row {
                width: parent.width
                height: 28

                Text {
                    text: "Settings"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 15
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Accent Color Card
            GlassCard {
                width: parent.width
                implicitHeight: 64

                Row {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 12

                    Column {
                        width: parent.width - 240
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: "Accent Color"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Text {
                            text: "Liquid Glass highlight and active tints"
                            color: Theme.textMuted
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                        }
                    }

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        Repeater {
                            model: root.accentColors

                            Rectangle {
                                width: 22
                                height: 22
                                radius: 11
                                color: modelData.color
                                border.color: (settings && settings.accentColor === modelData.color) ? "#FFFFFF" : "transparent"
                                border.width: 2

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: if (settings) settings.accentColor = modelData.color
                                }
                            }
                        }
                    }
                }
            }

            // System Integration Card (Startup & Hotkey)
            GlassCard {
                width: parent.width
                implicitHeight: 112

                Column {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    // Startup toggle
                    Row {
                        width: parent.width

                        Column {
                            width: parent.width - 100
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "Launch at Windows Startup"
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Text {
                                text: "Runs silently in the background on login"
                                color: Theme.textMuted
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                            }
                        }

                        GlassButton {
                            text: (settings && settings.autoStart) ? "Enabled" : "Disabled"
                            isAccent: settings && settings.autoStart
                            cornerRadius: 6
                            implicitHeight: 26
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: {
                                if (settings) settings.autoStart = !settings.autoStart
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.glassBorder }

                    // Global Hotkey info
                    Row {
                        width: parent.width

                        Column {
                            width: parent.width - 120
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "Global Shortcut"
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Text {
                                text: "Toggle Dynamic Island expansion instantly"
                                color: Theme.textMuted
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                            }
                        }

                        Rectangle {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            width: hotkeyTxt.implicitWidth + 12
                            height: 24
                            radius: 6
                            color: Theme.glassSurface
                            border.color: Theme.glassBorder

                            Text {
                                id: hotkeyTxt
                                anchors.centerIn: parent
                                text: "Ctrl + Alt + Space"
                                color: Theme.accent
                                font.family: Theme.fontMono
                                font.pixelSize: 11
                                font.bold: true
                            }
                        }
                    }
                }
            }

            // Clipboard Monitoring Card
            GlassCard {
                width: parent.width
                implicitHeight: 56

                Row {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 12

                    Column {
                        width: parent.width - 100
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: "Clipboard History Monitor"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Text {
                            text: "Auto-ignores password managers and sensitive fields"
                            color: Theme.textMuted
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                        }
                    }

                    GlassButton {
                        text: (settings && settings.clipboardEnabled) ? "Active" : "Paused"
                        isAccent: settings && settings.clipboardEnabled
                        cornerRadius: 6
                        implicitHeight: 26
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: {
                            if (settings) settings.clipboardEnabled = !settings.clipboardEnabled
                        }
                    }
                }
            }

            // Agent Hook Server Info
            GlassCard {
                width: parent.width
                implicitHeight: 70

                Column {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 4

                    Text {
                        text: "Claude Code / Codex Agent Hook Token"
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.bold: true
                    }

                    Row {
                        width: parent.width
                        spacing: 8

                        Rectangle {
                            width: parent.width - 90
                            height: 24
                            radius: 4
                            color: Qt.rgba(0.0, 0.0, 0.0, 0.4)
                            border.color: Theme.glassBorder

                            Text {
                                anchors.fill: parent
                                anchors.leftMargin: 6
                                text: settings ? settings.agentServerToken : ""
                                color: Theme.textSecondary
                                font.family: Theme.fontMono
                                font.pixelSize: 10
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        GlassButton {
                            text: "Copy"
                            cornerRadius: 4
                            implicitHeight: 24
                            implicitWidth: 80
                            onClicked: {
                                if (settings && appState) {
                                    appState.showToast("Agent token copied to clipboard", "success")
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
