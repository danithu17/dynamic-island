import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    readonly property var accentColors: [
        { name: "Electric Blue", color: "#0A84FF" },
        { name: "Liquid Purple", color: "#BF5AF2" },
        { name: "Emerald", color: "#30D158" },
        { name: "Sunset Orange", color: "#FF9F0A" },
        { name: "Cyber Pink", color: "#FF375F" },
        { name: "Ice Cyan", color: "#64D2FF" }
    ]

    readonly property var idleWidgetOptions: [
        { id: "clipboard", label: "Clipboard count" },
        { id: "media", label: "Now playing" },
        { id: "git", label: "Git branch" },
        { id: "pomodoro", label: "Focus timer" },
        { id: "mail", label: "Unread mail" },
        { id: "agent", label: "AI Agent" }
    ]

    ScrollView {
        anchors.fill: parent
        anchors.margins: 14
        clip: true

        Column {
            width: parent.width
            spacing: 12

            Text {
                text: "Aura Settings & Customization"
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 15
                font.bold: true
            }

            // ─────────────────────────────────────────────────
            // CARD 1: Idle Island Widget (Never Blank!)
            // ─────────────────────────────────────────────────
            GlassCard {
                width: parent.width
                implicitHeight: 74

                Column {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 6

                    Text {
                        text: "Idle Island Widget"
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.bold: true
                    }

                    Row {
                        spacing: 6

                        Repeater {
                            model: root.idleWidgetOptions

                            Rectangle {
                                width: idleOptTxt.implicitWidth + 12
                                height: 24
                                radius: 12
                                color: (settings && settings.idleWidget === modelData.id)
                                       ? Theme.accent
                                       : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                                border.color: (settings && settings.idleWidget === modelData.id)
                                              ? Theme.accent
                                              : Qt.rgba(1.0, 1.0, 1.0, 0.12)
                                border.width: 1

                                Text {
                                    id: idleOptTxt
                                    anchors.centerIn: parent
                                    text: modelData.label
                                    color: (settings && settings.idleWidget === modelData.id)
                                           ? "#FFFFFF"
                                           : Theme.textSecondary
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 10
                                    font.bold: true
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (settings) settings.idleWidget = modelData.id
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ─────────────────────────────────────────────────
            // CARD 2: Accent Colors
            // ─────────────────────────────────────────────────
            GlassCard {
                width: parent.width
                implicitHeight: 70

                Column {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 6

                    Text {
                        text: "Accent Color & Liquid Glow"
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.bold: true
                    }

                    Row {
                        spacing: 12

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

            // ─────────────────────────────────────────────────
            // CARD 3: Programmer Clipboard & Automation
            // ─────────────────────────────────────────────────
            GlassCard {
                width: parent.width
                implicitHeight: 120

                Column {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    // Auto-paste toggle
                    Row {
                        width: parent.width

                        Column {
                            width: parent.width - 100
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 1

                            Text {
                                text: "Auto-Paste on Click / (1-9)"
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Text {
                                text: "Synthesizes Ctrl+V to target app upon user action"
                                color: Theme.textMuted
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                            }
                        }

                        GlassButton {
                            text: (settings && settings.autoPaste) ? "ON" : "OFF"
                            isAccent: settings && settings.autoPaste
                            cornerRadius: 6
                            implicitHeight: 24
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: if (settings) settings.autoPaste = !settings.autoPaste
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.glassBorder }

                    // Paste Stack mode toggle
                    Row {
                        width: parent.width

                        Column {
                            width: parent.width - 100
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 1

                            Text {
                                text: "Paste Stack Queue Mode"
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Text {
                                text: "Ctrl+Alt+V pastes next queued item in order"
                                color: Theme.textMuted
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                            }
                        }

                        GlassButton {
                            text: (settings && settings.pasteStackEnabled) ? "ON" : "OFF"
                            isAccent: settings && settings.pasteStackEnabled
                            cornerRadius: 6
                            implicitHeight: 24
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: if (settings) settings.pasteStackEnabled = !settings.pasteStackEnabled
                        }
                    }
                }
            }

            // ─────────────────────────────────────────────────
            // CARD 4: Global Shortcuts & Startup
            // ─────────────────────────────────────────────────
            GlassCard {
                width: parent.width
                implicitHeight: 130

                Column {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    // Startup toggle
                    Row {
                        width: parent.width

                        Column {
                            width: parent.width - 100
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 1

                            Text {
                                text: "Launch at Windows Startup"
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Text {
                                text: "Runs in background without taskbar icon"
                                color: Theme.textMuted
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                            }
                        }

                        GlassButton {
                            text: (settings && settings.autoStart) ? "Enabled" : "Disabled"
                            isAccent: settings && settings.autoStart
                            cornerRadius: 6
                            implicitHeight: 24
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: if (settings) settings.autoStart = !settings.autoStart
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.glassBorder }

                    // Hotkeys list
                    Row {
                        width: parent.width
                        spacing: 12

                        Text {
                            text: "Hotkeys:"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "Toggle: Ctrl+Alt+Space | Quick Note: Ctrl+Alt+N | Stack: Ctrl+Alt+V"
                            color: Theme.accent
                            font.family: Theme.fontMono
                            font.pixelSize: 9
                            font.bold: true
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            // ─────────────────────────────────────────────────
            // CARD 5: Agent Hook Server Info
            // ─────────────────────────────────────────────────
            GlassCard {
                width: parent.width
                implicitHeight: 70

                Column {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 4

                    Text {
                        text: "Claude Code / Cursor Agent Hook Token (127.0.0.1:19827)"
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
