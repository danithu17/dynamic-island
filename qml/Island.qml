import QtQuick
import QtQuick.Controls
import "components"
import "tabs"
import "."

Item {
    id: root

    property string stateName: appState ? appState.islandState : "idle"
    property bool isHovered: false

    // Dimensions morphing based on state
    readonly property int targetWidth: {
        if (stateName === "idle") return root.isHovered ? 132 : 120
        if (stateName === "compact") return 260
        if (stateName === "expanded") return 560
        if (stateName === "drop") return 400
        return 120
    }

    readonly property int targetHeight: {
        if (stateName === "idle") return root.isHovered ? 34 : 32
        if (stateName === "compact") return 36
        if (stateName === "expanded") return 340
        if (stateName === "drop") return 140
        return 32
    }

    readonly property int targetRadius: {
        if (stateName === "idle") return 16
        if (stateName === "compact") return 18
        if (stateName === "expanded") return 32
        if (stateName === "drop") return 24
        return 16
    }

    width: targetWidth
    height: targetHeight

    Behavior on width {
        NumberAnimation { duration: Theme.animDurationNormal; easing.type: Theme.animEasingSpring }
    }
    Behavior on height {
        NumberAnimation { duration: Theme.animDurationNormal; easing.type: Theme.animEasingSpring }
    }

    // Glass Pill Background
    GlassPill {
        id: pillBg
        anchors.fill: parent
        cornerRadius: root.targetRadius
        activeGlow: root.stateName === "drop"

        Behavior on cornerRadius {
            NumberAnimation { duration: Theme.animDurationNormal; easing.type: Theme.animEasingSpring }
        }
    }

    // ==========================================
    // 1. IDLE STATE CONTENT (~120x32)
    // ==========================================
    Item {
        id: idleView
        anchors.fill: parent
        opacity: (root.stateName === "idle") ? 1.0 : 0.0
        visible: opacity > 0
        scale: (root.stateName === "idle") ? 1.0 : 0.85

        Behavior on opacity {
            NumberAnimation { duration: Theme.animDurationFast }
        }
        Behavior on scale {
            NumberAnimation { duration: Theme.animDurationFast; easing.type: Theme.animEasing }
        }

        Row {
            anchors.centerIn: parent
            spacing: 10

            // Ambient status dot / camera cutout
            Rectangle {
                width: 7
                height: 7
                radius: 3.5
                color: (appState && appState.agentStatus === "needs_approval") ? Theme.warning : ((appState && appState.isPlaying) ? Theme.accent : "#050608")
                border.color: Qt.rgba(1.0, 1.0, 1.0, 0.15)
                border.width: 1
                anchors.verticalCenter: parent.verticalCenter

                SequentialAnimation on opacity {
                    running: appState && (appState.isPlaying || appState.agentStatus === "working")
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.4; duration: 800 }
                    NumberAnimation { to: 1.0; duration: 800 }
                }
            }

            // Compact dynamic pill pill-sensor
            Rectangle {
                width: 44
                height: 10
                radius: 5
                color: "#08090D"
                border.color: Qt.rgba(1.0, 1.0, 1.0, 0.1)
                border.width: 0.8
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onEntered: root.isHovered = true
            onExited: root.isHovered = false
            onClicked: {
                if (appState) appState.setIslandState("compact")
            }
            onDoubleClicked: {
                if (appState) appState.toggleExpanded()
            }
        }
    }

    // ==========================================
    // 2. COMPACT STATE CONTENT (~260x36)
    // ==========================================
    Item {
        id: compactView
        anchors.fill: parent
        anchors.margins: 4
        opacity: (root.stateName === "compact") ? 1.0 : 0.0
        visible: opacity > 0
        scale: (root.stateName === "compact") ? 1.0 : 0.9

        Behavior on opacity {
            NumberAnimation { duration: Theme.animDurationFast }
        }
        Behavior on scale {
            NumberAnimation { duration: Theme.animDurationFast; easing.type: Theme.animEasing }
        }

        Row {
            anchors.fill: parent
            spacing: 8

            // Left: Mail Badge or Music Icon
            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                IconImage {
                    source: (appState && appState.isPlaying) ? "qrc:/resources/icons/media.svg" : "qrc:/resources/icons/mail.svg"
                    size: 14
                    color: Theme.accent
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    visible: appState && appState.unreadMailCount > 0
                    text: appState ? appState.unreadMailCount : ""
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Center: Music / Timer / Status Info
            Item {
                width: parent.width - 90
                height: parent.height

                Text {
                    anchors.centerIn: parent
                    text: {
                        if (appState && appState.isPlaying && appState.nowPlayingTitle !== "") {
                            return appState.nowPlayingTitle
                        }
                        if (appState && appState.pomodoroDisplay !== "") {
                            return "Focus: " + appState.pomodoroDisplay
                        }
                        return "Aura Island"
                    }
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            // Right: Animated Equalizer or Pomodoro indicator
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                EqualizerBars {
                    visible: appState && appState.isPlaying
                    isPlaying: appState ? appState.isPlaying : false
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Expand button
                Rectangle {
                    width: 18
                    height: 18
                    radius: 9
                    color: Qt.rgba(1.0, 1.0, 1.0, 0.1)
                    anchors.verticalCenter: parent.verticalCenter

                    IconImage {
                        anchors.centerIn: parent
                        source: "qrc:/resources/icons/dev.svg"
                        size: 9
                        color: Theme.textPrimary
                    }
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (appState) appState.toggleExpanded()
            }
        }
    }

    // ==========================================
    // 3. EXPANDED STATE CONTENT (~560x340)
    // ==========================================
    Item {
        id: expandedView
        anchors.fill: parent
        anchors.margins: 10
        opacity: (root.stateName === "expanded") ? 1.0 : 0.0
        visible: opacity > 0
        scale: (root.stateName === "expanded") ? 1.0 : 0.95

        Behavior on opacity {
            NumberAnimation { duration: Theme.animDurationNormal }
        }
        Behavior on scale {
            NumberAnimation { duration: Theme.animDurationNormal; easing.type: Theme.animEasingSpring }
        }

        Column {
            anchors.fill: parent
            spacing: 8

            // Top: Liquid Sliding Tab Bar & Close/Collapse Button
            Row {
                width: parent.width
                height: 38
                spacing: 8

                SlidingTabBar {
                    width: parent.width - 44
                    activeTab: appState ? appState.currentTab : "shelf"
                    onTabSelected: function(tabId) {
                        if (appState) appState.setCurrentTab(tabId)
                    }
                }

                // Collapse button
                GlassButton {
                    iconSource: "qrc:/resources/icons/close.svg"
                    cornerRadius: 19
                    implicitWidth: 38
                    implicitHeight: 38
                    onClicked: {
                        if (appState) appState.collapseToIdle()
                    }
                }
            }

            // Tab Content Host with smooth fade
            Rectangle {
                width: parent.width
                height: parent.height - 46
                radius: 20
                color: Qt.rgba(0.04, 0.04, 0.06, 0.45)
                border.color: Qt.rgba(1.0, 1.0, 1.0, 0.08)
                clip: true

                ShelfTab {
                    anchors.fill: parent
                    visible: appState && appState.currentTab === "shelf"
                }

                MailTab {
                    anchors.fill: parent
                    visible: appState && appState.currentTab === "mail"
                }

                ClipboardTab {
                    anchors.fill: parent
                    visible: appState && appState.currentTab === "clipboard"
                }

                NotesTab {
                    anchors.fill: parent
                    visible: appState && appState.currentTab === "notes"
                }

                DevTab {
                    anchors.fill: parent
                    visible: appState && appState.currentTab === "dev"
                }

                MediaTab {
                    anchors.fill: parent
                    visible: appState && appState.currentTab === "media"
                }

                SettingsTab {
                    anchors.fill: parent
                    visible: appState && appState.currentTab === "settings"
                }
            }
        }

        // Error & Toast Notifications inside Island
        ToastNotification {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    // ==========================================
    // 4. DROP STATE CONTENT (~400x140)
    // ==========================================
    Item {
        id: dropView
        anchors.fill: parent
        opacity: (root.stateName === "drop") ? 1.0 : 0.0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation { duration: Theme.animDurationFast }
        }

        DropZone {
            anchors.fill: parent
        }
    }
}
