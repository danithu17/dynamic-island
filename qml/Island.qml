import QtQuick
import QtQuick.Controls
import "components"
import "tabs"
import "."

Item {
    id: root

    property string stateName: appState ? appState.islandState : "idle"
    property bool isHovered: false

    // ─────────────────────────────────────────────────────────
    // Target dimensions per state
    // ─────────────────────────────────────────────────────────
    readonly property int targetWidth: {
        if (stateName === "idle")     return 120
        if (stateName === "compact")  return 260
        if (stateName === "expanded") return 560
        if (stateName === "drop")     return 400
        return 120
    }

    readonly property int targetHeight: {
        if (stateName === "idle")     return 32
        if (stateName === "compact")  return 36
        if (stateName === "expanded") return 340
        if (stateName === "drop")     return 140
        return 32
    }

    readonly property int targetRadius: {
        if (stateName === "idle")     return 16
        if (stateName === "compact")  return 18
        if (stateName === "expanded") return 32
        if (stateName === "drop")     return 24
        return 16
    }

    // ─────────────────────────────────────────────────────────
    // Dimensions fill parent (Window)
    // ─────────────────────────────────────────────────────────
    width:  parent ? parent.width : targetWidth
    height: parent ? parent.height : targetHeight

    // ─────────────────────────────────────────────────────────
    // Glass Pill Background
    // ─────────────────────────────────────────────────────────
    GlassPill {
        id: pillBg
        anchors.fill: parent
        cornerRadius: root.targetRadius
        activeGlow: root.stateName === "drop"

        Behavior on cornerRadius {
            SmoothedAnimation {
                duration: Theme.animDurationNormal
                easing.type: Easing.OutCubic
            }
        }
    }

    // ─────────────────────────────────────────────────────────
    // Pulsing outer ring for drop zone
    // ─────────────────────────────────────────────────────────
    Rectangle {
        id: dropPulseRing
        anchors.fill: parent
        anchors.margins: -6
        radius: root.targetRadius + 6
        color: "transparent"
        border.color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.5)
        border.width: 1.5
        visible: root.stateName === "drop"

        SequentialAnimation on opacity {
            running: root.stateName === "drop"
            loops:   Animation.Infinite
            NumberAnimation { to: 0.15; duration: 750 }
            NumberAnimation { to: 1.0;  duration: 750 }
        }
    }

    // ─────────────────────────────────────────────────────────
    // 1. IDLE STATE (~120x32)
    // ─────────────────────────────────────────────────────────
    Item {
        id: idleView
        anchors.fill: parent
        opacity: (root.stateName === "idle") ? 1.0 : 0.0
        visible: opacity > 0.01
        scale:   (root.stateName === "idle") ? 1.0 : 0.80

        Behavior on opacity {
            NumberAnimation { duration: Theme.animDurationFast; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            SpringAnimation { spring: 4.0; damping: 0.60; epsilon: 0.001 }
        }

        Row {
            anchors.centerIn: parent
            spacing: 10

            // Status / camera-cutout dot
            Rectangle {
                id: statusDot
                width: 7; height: 7; radius: 3.5
                color: (appState && appState.agentStatus === "needs_approval")
                       ? Theme.warning
                       : ((appState && appState.isPlaying) ? Theme.accent : "#050608")
                border.color: Qt.rgba(1.0, 1.0, 1.0, 0.15)
                border.width: 1
                anchors.verticalCenter: parent.verticalCenter

                SequentialAnimation on opacity {
                    running: appState && (appState.isPlaying || appState.agentStatus === "working")
                    loops:   Animation.Infinite
                    NumberAnimation { to: 0.35; duration: 900; easing.type: Easing.InOutSine }
                    NumberAnimation { to: 1.0;  duration: 900; easing.type: Easing.InOutSine }
                }

                // Color transition for status
                Behavior on color {
                    ColorAnimation { duration: Theme.animDurationFast }
                }
            }

            // Pill notch sensor strip
            Rectangle {
                width: 44; height: 10; radius: 5
                color: "#07080C"
                border.color: Qt.rgba(1.0, 1.0, 1.0, 0.10)
                border.width: 0.8
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onEntered:      root.isHovered = true
            onExited:       root.isHovered = false
            onClicked:      if (appState) appState.setIslandState("compact")
            onDoubleClicked: if (appState) appState.toggleExpanded()
        }
    }

    // ─────────────────────────────────────────────────────────
    // 2. COMPACT STATE (~260x36)
    // ─────────────────────────────────────────────────────────
    Item {
        id: compactView
        anchors.fill: parent
        anchors.margins: 4
        opacity: (root.stateName === "compact") ? 1.0 : 0.0
        visible: opacity > 0.01
        scale:   (root.stateName === "compact") ? 1.0 : 0.88

        Behavior on opacity {
            NumberAnimation { duration: Theme.animDurationFast; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            SpringAnimation { spring: 4.5; damping: 0.65; epsilon: 0.001 }
        }

        Row {
            anchors.fill: parent
            spacing: 8

            // Left: icon + badge
            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                IconImage {
                    source: (appState && appState.isPlaying)
                             ? "qrc:/resources/icons/media.svg"
                             : "qrc:/resources/icons/mail.svg"
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

            // Centre: now-playing / focus timer / name
            Item {
                width: parent.width - 90
                height: parent.height

                Text {
                    id: compactLabel
                    anchors.centerIn: parent
                    text: {
                        if (appState && appState.isPlaying && appState.nowPlayingTitle !== "")
                            return appState.nowPlayingTitle
                        if (appState && appState.pomodoroDisplay !== "")
                            return "Focus: " + appState.pomodoroDisplay
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

            // Right: equalizer bars + expand pill
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                EqualizerBars {
                    visible: appState && appState.isPlaying
                    isPlaying: appState ? appState.isPlaying : false
                    anchors.verticalCenter: parent.verticalCenter
                }

                Rectangle {
                    width: 18; height: 18; radius: 9
                    color: Qt.rgba(1.0, 1.0, 1.0, 0.10)
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
            onClicked: if (appState) appState.toggleExpanded()
        }
    }

    // ─────────────────────────────────────────────────────────
    // 3. EXPANDED STATE (~560x340)
    // ─────────────────────────────────────────────────────────
    Item {
        id: expandedView
        anchors.fill: parent
        anchors.margins: 10
        opacity: (root.stateName === "expanded") ? 1.0 : 0.0
        visible: opacity > 0.01

        // Enter: scale + fade from 0.93
        property real contentScale: (root.stateName === "expanded") ? 1.0 : 0.93

        Behavior on opacity {
            NumberAnimation {
                duration: Theme.animDurationNormal
                easing.type: Easing.OutCubic
            }
        }

        // We scale the Column directly
        Column {
            id: expandedColumn
            anchors.fill: parent
            spacing: 8

            scale: expandedView.contentScale
            transformOrigin: Item.Top

            Behavior on scale {
                SpringAnimation { spring: 3.5; damping: 0.60; epsilon: 0.001 }
            }

            // ── Tab bar + collapse button ──
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

                GlassButton {
                    iconSource: "qrc:/resources/icons/close.svg"
                    cornerRadius: 19
                    implicitWidth: 38
                    implicitHeight: 38
                    onClicked: if (appState) appState.collapseToIdle()
                }
            }

            // ── Tab content area ──
            Rectangle {
                id: contentArea
                width: parent.width
                height: parent.height - 46
                radius: 20
                color: Qt.rgba(0.04, 0.04, 0.06, 0.45)
                border.color: Qt.rgba(1.0, 1.0, 1.0, 0.08)
                clip: true

                // Each tab fades + scales in individually
                ShelfTab     { anchors.fill: parent; visible: appState && appState.currentTab === "shelf" }
                MailTab      { anchors.fill: parent; visible: appState && appState.currentTab === "mail" }
                ClipboardTab { anchors.fill: parent; visible: appState && appState.currentTab === "clipboard" }
                NotesTab     { anchors.fill: parent; visible: appState && appState.currentTab === "notes" }
                DevTab       { anchors.fill: parent; visible: appState && appState.currentTab === "dev" }
                MediaTab     { anchors.fill: parent; visible: appState && appState.currentTab === "media" }
                SettingsTab  { anchors.fill: parent; visible: appState && appState.currentTab === "settings" }

                // Soft top-edge specular line inside content panel
                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 1
                    color: Qt.rgba(1.0, 1.0, 1.0, 0.07)
                }
            }
        }

        // Toast notifications
        ToastNotification {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    // ─────────────────────────────────────────────────────────
    // 4. DROP STATE (~400x140)
    // ─────────────────────────────────────────────────────────
    Item {
        id: dropView
        anchors.fill: parent
        opacity: (root.stateName === "drop") ? 1.0 : 0.0
        visible: opacity > 0.01
        scale:   (root.stateName === "drop") ? 1.0 : 0.90

        Behavior on opacity {
            NumberAnimation { duration: Theme.animDurationFast; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            SpringAnimation { spring: 4.0; damping: 0.65; epsilon: 0.001 }
        }

        DropZone {
            anchors.fill: parent
        }
    }
}
