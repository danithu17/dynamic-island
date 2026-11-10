import QtQuick
import ".."

Item {
    id: root
    property string activeTab: appState ? appState.currentTab : "home"
    signal tabSelected(string tabId)

    implicitHeight: 38
    implicitWidth: 480

    readonly property var tabs: [
        { id: "home", label: "Home", icon: "qrc:/resources/icons/home.svg" },
        { id: "shelf", label: "Tray", icon: "qrc:/resources/icons/shelf.svg" },
        { id: "clipboard", label: "Clips", icon: "qrc:/resources/icons/clipboard.svg" },
        { id: "notes", label: "Notes", icon: "qrc:/resources/icons/notes.svg" },
        { id: "media", label: "Media", icon: "qrc:/resources/icons/media.svg" },
        { id: "ai", label: "AI", icon: "qrc:/resources/icons/sparkles.svg" },
        { id: "games", label: "Play", icon: "qrc:/resources/icons/gamepad.svg" },
        { id: "focus", label: "Focus", icon: "qrc:/resources/icons/moon.svg" },
        { id: "mail", label: "Mail", icon: "qrc:/resources/icons/mail.svg", badge: (appState ? appState.unreadMailCount : 0) },
        { id: "dev", label: "Dev", icon: "qrc:/resources/icons/dev.svg", badge: (appState ? (appState.agentStatus === "needs_approval" ? "!" : "") : "") },
        { id: "settings", label: "", icon: "qrc:/resources/icons/settings.svg" }
    ]

    function getActiveIndex() {
        for (var i = 0; i < tabs.length; i++) {
            if (tabs[i].id === activeTab) return i;
        }
        return 0;
    }

    Rectangle {
        id: barBackground
        anchors.fill: parent
        radius: 19
        color: Qt.rgba(0.0, 0.0, 0.0, 0.40)
        border.color: Theme.glassBorder
        border.width: 1

        Row {
            anchors.fill: parent
            anchors.leftMargin: 4
            anchors.rightMargin: 4
            spacing: 3

            // ─────────────────────────────────────────────────
            // FAR-LEFT QUICK LAUNCH AREA
            // ─────────────────────────────────────────────────
            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 3

                // YouTube Shortcut
                Rectangle {
                    width: 24
                    height: 24
                    radius: 12
                    color: ytMouse.containsMouse ? "#E50914" : Qt.rgba(1.0, 0.0, 0.0, 0.20)
                    border.color: ytMouse.containsMouse ? "#FF4D4D" : Qt.rgba(1.0, 0.2, 0.2, 0.35)
                    border.width: 1
                    scale: ytMouse.pressed ? 0.92 : (ytMouse.containsMouse ? 1.08 : 1.0)
                    Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.6 } }
                    Behavior on color { ColorAnimation { duration: 120 } }

                    IconImage {
                        anchors.centerIn: parent
                        source: "qrc:/resources/icons/youtube.svg"
                        size: 13
                        color: "#FFFFFF"
                    }

                    MouseArea {
                        id: ytMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            Qt.openUrlExternally("https://www.youtube.com")
                            if (appState) appState.showToast("Opening YouTube...", "info")
                        }
                    }
                }

                // Add Custom Shortcut Button ("+")
                Rectangle {
                    width: 20
                    height: 20
                    radius: 10
                    anchors.verticalCenter: parent.verticalCenter
                    color: addMouse.containsMouse ? Theme.glassSurfaceHover : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                    border.color: addMouse.containsMouse ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.12)
                    border.width: 1
                    scale: addMouse.pressed ? 0.92 : (addMouse.containsMouse ? 1.08 : 1.0)
                    Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.6 } }

                    IconImage {
                        anchors.centerIn: parent
                        source: "qrc:/resources/icons/plus.svg"
                        size: 9
                        color: Theme.textSecondary
                    }

                    MouseArea {
                        id: addMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (appState) {
                                appState.setCurrentTab("settings")
                                appState.showToast("Shortcut Launcher: Configure custom apps in Settings", "info")
                            }
                        }
                    }
                }

                // Subtle vertical divider line
                Rectangle {
                    width: 1
                    height: 16
                    radius: 0.5
                    color: Qt.rgba(1.0, 1.0, 1.0, 0.14)
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // ─────────────────────────────────────────────────
            // DOCK TABS AREA
            // ─────────────────────────────────────────────────
            Item {
                id: tabsArea
                height: parent.height
                width: parent.width - 62

                // Liquid Mercury Sliding Glass Indicator
                Rectangle {
                    id: indicator
                    property int targetIdx: root.getActiveIndex()
                    property Item targetTab: tabRepeater.itemAt(targetIdx)

                    x: targetTab ? targetTab.x + 2 : 2
                    y: 3
                    width: targetTab ? targetTab.width - 4 : 34
                    height: parent.height - 6
                    radius: 15

                    gradient: Gradient {
                        GradientStop { position: 0.0; color: (Theme && Theme.accent) ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.38) : "#330A84FF" }
                        GradientStop { position: 1.0; color: (Theme && Theme.accent) ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.18) : "#180A84FF" }
                    }
                    border.color: (Theme && Theme.accent) ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.65) : "#990A84FF"
                    border.width: 1

                    Behavior on x {
                        SpringAnimation {
                            spring:  4.2
                            damping: 0.60
                            epsilon: 0.5
                        }
                    }
                    Behavior on width {
                        SpringAnimation {
                            spring:  4.2
                            damping: 0.60
                            epsilon: 0.5
                        }
                    }
                }

                Row {
                    id: tabRow
                    anchors.fill: parent
                    spacing: 1

                    Repeater {
                        id: tabRepeater
                        model: root.tabs

                        Item {
                            id: tabItem
                            width: (tabRow.width - ((root.tabs.length - 1) * tabRow.spacing)) / root.tabs.length
                            height: tabRow.height

                            readonly property bool isActive: root.activeTab === modelData.id

                            Row {
                                anchors.centerIn: parent
                                spacing: 3

                                IconImage {
                                    source: modelData.icon
                                    size: 14
                                    color: tabItem.isActive ? Theme.textPrimary : (tabMouse.containsMouse ? "#FFFFFF" : Theme.textSecondary)
                                    anchors.verticalCenter: parent.verticalCenter
                                    scale: tabMouse.pressed ? 0.90 : (tabMouse.containsMouse ? 1.10 : 1.0)
                                    Behavior on scale { SpringAnimation { spring: 5.0; damping: 0.55 } }
                                    Behavior on color { ColorAnimation { duration: 120 } }
                                }

                                Text {
                                    visible: tabItem.isActive && modelData.label !== ""
                                    text: modelData.label
                                    color: Theme.textPrimary
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 9
                                    font.weight: Font.DemiBold
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                // Notification Badge
                                Rectangle {
                                    visible: !!modelData.badge && modelData.badge !== 0 && modelData.badge !== ""
                                    width: Math.max(badgeText.implicitWidth + 4, 12)
                                    height: 12
                                    radius: 6
                                    color: modelData.badge === "!" ? Theme.warning : Theme.danger
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        id: badgeText
                                        anchors.centerIn: parent
                                        text: modelData.badge
                                        color: "#FFFFFF"
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 8
                                        font.bold: true
                                    }
                                }
                            }

                            // Active Tab Bottom Dot
                            Rectangle {
                                visible: tabItem.isActive
                                width: 3
                                height: 3
                                radius: 1.5
                                color: "#FFFFFF"
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: 2
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            MouseArea {
                                id: tabMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                hoverEnabled: true
                                onClicked: {
                                    root.tabSelected(modelData.id)
                                    if (appState) appState.setCurrentTab(modelData.id)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
